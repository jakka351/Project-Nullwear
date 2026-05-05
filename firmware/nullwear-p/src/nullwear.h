/*
 * Project NULLWEAR — Application-core common definitions
 *
 * Copyright (c) 2026 Benjamin Jack Leighton, Tester Present Specialist
 * Automotive Solutions.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef NULLWEAR_APP_H
#define NULLWEAR_APP_H

#include <stdint.h>
#include <stdbool.h>

/* Statistics snapshot received from the network core. Mirrors the
 * radio jammer's internal struct verbatim. */
typedef struct {
    uint32_t pkts_received_total;
    uint32_t pkts_oui_matched;
    uint32_t pkts_jammed;
    uint32_t errors_tx_late;
    uint32_t errors_rx_aborted;
    uint32_t channel_hops;
} nullwear_stats_t;

/* Battery state-of-charge tiers used to drive the LED. */
typedef enum {
    BAT_OK   = 0,   /* >50%   — green */
    BAT_LOW  = 1,   /* 20-50% — amber */
    BAT_CRIT = 2,   /* <20%   — red blink */
    BAT_CHG  = 3,   /* charging — pulsing green */
    BAT_FAULT = 4,  /* fault  — red rapid */
} bat_state_t;

#endif /* NULLWEAR_APP_H */
