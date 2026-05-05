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
 * selective per-packet annihilation of BLE advertising packets carrying
 * the Axon Enterprise OUI 00:25:DF.
 */

#ifndef NULLWEAR_RADIO_JAMMER_H
#define NULLWEAR_RADIO_JAMMER_H

#include <stdint.h>
#include <stdbool.h>

/* Target OUI: Taser International / Axon Enterprise. */
#define NULLWEAR_TARGET_OUI_B0    0x00U
#define NULLWEAR_TARGET_OUI_B1    0x25U
#define NULLWEAR_TARGET_OUI_B2    0xDFU

/* BLE primary advertising channels (BLE 5.x §3.2.1). */
#define BLE_ADV_CH_37_FREQ_MHZ    2402U
#define BLE_ADV_CH_38_FREQ_MHZ    2426U
#define BLE_ADV_CH_39_FREQ_MHZ    2480U

/* Logical channel index, matching nRF FREQUENCY register encoding. */
#define BLE_ADV_CH_37_FREQ_REG    2U     /* 2400 + 2  = 2402 MHz */
#define BLE_ADV_CH_38_FREQ_REG    26U    /* 2400 + 26 = 2426 MHz */
#define BLE_ADV_CH_39_FREQ_REG    80U    /* 2400 + 80 = 2480 MHz */

/* Whitening initialisation values (BLE 5.x §3.2 - LFSR seeded with channel
 * index ORed with bit 6 set). */
#define BLE_WHITEN_CH_37          0x65U
#define BLE_WHITEN_CH_38          0x66U
#define BLE_WHITEN_CH_39          0x67U

/* BLE advertising-channel access address (BLE 5.x §3.2). */
#define BLE_ADV_ACCESS_ADDR       0x8E89BED6UL

/* Channel-hop dwell time (microseconds). 80 ms across the 3 channels gives
 * adequate coverage of the ~10 ms advertising interval typical of Axon kit. */
#define NULLWEAR_CH_DWELL_US      80000U

/* Statistics counters published to the application core via IPC. */
typedef struct {
    uint32_t pkts_received_total;
    uint32_t pkts_oui_matched;
    uint32_t pkts_jammed;
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
