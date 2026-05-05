/*
 * Project NULLWEAR — Network-core radio jammer interface
 *
 * Copyright (c) 2026 Benjamin Jack Leighton, Tester Present Specialist
 * Automotive Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * This module owns the nRF5340 RADIO peripheral on the network core.
 * It is responsible for the entire RX/TX pipeline that performs the
 * selective per-packet annihilation of BLE advertising packets emitted
 * by Axon Enterprise law-enforcement equipment.
 *
 * v1.1 — Dual-signature matching
 * ------------------------------
 *
 * v1.0 matched only on MAC OUI 00:25:DF.
 *
 * v1.1 adds a SECOND match path on the Axon Public Safety BLE
 * Service-Data UUID 0xFE6B (registered to Axon by the Bluetooth SIG).
 * This second path catches Axon devices that broadcast in their
 * "non-deployed / docked" state with a sanitised AdvA (e.g. all-zero
 * MAC) but still emit Axon service data — a behaviour empirically
 * observed in the Threat Validation dataset (see companion PDF).
 *
 * Detection sequence:
 *
 *   1. ADDRESS event fires → BCSTART begins counting bits of PDU.
 *   2. BCMATCH fires at bit 64 (header + full AdvA): inspect MAC OUI.
 *      → If 00:25:DF, jam immediately. Done.
 *   3. Otherwise: re-arm BCC for bit 128 (header + AdvA + first
 *      8 bytes of AdvData), wait for second BCMATCH.
 *   4. BCMATCH fires at bit 128: scan AdvData for the 3-byte pattern
 *      0x16 0x6B 0xFE (Service Data 16-bit AD type + UUID 0xFE6B
 *      little-endian).
 *      → If present, jam immediately. Done.
 *   5. Otherwise: not an Axon broadcast, let packet complete normally.
 *
 * Timing (BLE 1 Mbps PHY):
 *
 *   T0      Packet preamble starts on air
 *   T+ 40   AccessAddress complete; ADDRESS fires; BCSTART
 *   T+104   BCC=64 fires (Phase 1 — OUI check)
 *   T+~115  Jam-pulse begins on air (if OUI match)
 *   T+168   BCC=128 fires (Phase 2 — UUID check)
 *   T+~180  Jam-pulse begins on air (if UUID match)
 *
 * The CRC region of an empirically-observed Axon advertisement starts
 * at T+328 µs (28 B AdvData) to T+344 µs (30 B AdvData) and ends at
 * T+352 µs to T+368 µs respectively. Both jam-start times leave
 * comfortable margin for the 38-byte (304 µs) jam pulse to overlap
 * the CRC region.
 */

#ifndef NULLWEAR_RADIO_JAMMER_H
#define NULLWEAR_RADIO_JAMMER_H

#include <stdint.h>
#include <stdbool.h>

/* === Phase 1: MAC OUI match === */

/* Target OUI: Taser International / Axon Enterprise. */
#define NULLWEAR_TARGET_OUI_B0    0x00U
#define NULLWEAR_TARGET_OUI_B1    0x25U
#define NULLWEAR_TARGET_OUI_B2    0xDFU

/* === Phase 2: BLE Service Data UUID match === */

/* 16-bit BLE service UUID registered to Axon Public Safety with the
 * Bluetooth SIG. This UUID appears in a Service Data 16-bit AD
 * structure (AD type 0x16) inside Axon advertisements regardless of
 * whether the device is in deployed or docked/non-deployed state.
 *
 * On the air, the UUID is little-endian: byte sequence 0x6B 0xFE
 * follows the 0x16 type byte.
 */
#define NULLWEAR_AXON_SERVICE_UUID_LO   0x6BU   /* low byte (on air first) */
#define NULLWEAR_AXON_SERVICE_UUID_HI   0xFEU   /* high byte */
#define NULLWEAR_AD_TYPE_SERVICE_DATA_16BIT  0x16U

/* === Phase 2 secondary: 128-bit Service UUIDs (informational) ===
 *
 * Two Axon-specific 128-bit service UUIDs have been observed in the
 * field:
 *   4d455452-4f50-4f4c-4953-444556494345  ("METROPOLISDEVICE")
 *      — typical of deployed body cameras
 *   9ec5d2b8-8f51-4dea-9cd3-f3dea220b5e0
 *      — observed on docked/zero-MAC devices
 *
 * These are NOT used by the v1.1 matcher because the 16-bit UUID
 * 0xFE6B is present on every observed Axon advertisement (deployed
 * AND docked), and matching on 16 bits is faster + simpler than 128.
 * The 128-bit UUIDs are listed here for diagnostic / reference value
 * and may be used by a future v1.2 matcher if more selectivity is
 * required.
 */

/* === BLE primary advertising channels (BLE 5.x §3.2.1) === */

#define BLE_ADV_CH_37_FREQ_MHZ    2402U
#define BLE_ADV_CH_38_FREQ_MHZ    2426U
#define BLE_ADV_CH_39_FREQ_MHZ    2480U

/* Logical channel index, matching nRF FREQUENCY register encoding. */
#define BLE_ADV_CH_37_FREQ_REG    2U     /* 2400 + 2  = 2402 MHz */
#define BLE_ADV_CH_38_FREQ_REG    26U    /* 2400 + 26 = 2426 MHz */
#define BLE_ADV_CH_39_FREQ_REG    80U    /* 2400 + 80 = 2480 MHz */

/* Whitening initialisation values (Nordic register convention:
 * 0x40 | channel_index). */
#define BLE_WHITEN_CH_37          0x65U
#define BLE_WHITEN_CH_38          0x66U
#define BLE_WHITEN_CH_39          0x67U

/* BLE advertising-channel access address (BLE 5.x §3.2). */
#define BLE_ADV_ACCESS_ADDR       0x8E89BED6UL

/* Bit-counter trigger points for the two-phase matcher (in bits past
 * end-of-AccessAddress). */
#define NULLWEAR_BCC_PHASE_OUI    64U   /* header (16) + AdvA (48) */
#define NULLWEAR_BCC_PHASE_UUID   128U  /* + 8 B (64 bits) of AdvData */

/* Channel-hop dwell time (microseconds). */
#define NULLWEAR_CH_DWELL_US      80000U

/* === Configuration knobs (build-time) === */

/* Set to 0 to disable Phase 2 matching and revert to v1.0 behaviour
 * (OUI-only). Default: 1 (Phase 2 enabled). */
#ifndef NULLWEAR_ENABLE_PHASE_2
#define NULLWEAR_ENABLE_PHASE_2   1
#endif

/* === Statistics counters published to the application core via IPC === */

typedef struct {
    uint32_t pkts_received_total;
    uint32_t pkts_oui_matched;          /* Phase 1 hits (deployed Axon kit) */
    uint32_t pkts_uuid_matched;         /* Phase 2 hits (docked / privacy-mode kit) */
    uint32_t pkts_jammed;               /* Total jam events (sum of above) */
    uint32_t errors_tx_late;
    uint32_t errors_rx_aborted;
    uint32_t channel_hops;
} nullwear_radio_stats_t;

/**
 * Initialise the RADIO peripheral and begin scanning the three BLE
 * primary advertising channels. Returns immediately; the radio runs
 * in interrupt context and does not require the application core's
 * involvement once started.
 */
void radio_jammer_start(void);

/**
 * Stop the radio. After this call returns, the peripheral is disabled
 * and consumes only leakage current. Used during deep-sleep transitions.
 */
void radio_jammer_stop(void);

/**
 * Read a snapshot of the running statistics. Safe to call from any
 * priority lower than the RADIO IRQ.
 */
void radio_jammer_get_stats(nullwear_radio_stats_t *out);

#endif /* NULLWEAR_RADIO_JAMMER_H */
