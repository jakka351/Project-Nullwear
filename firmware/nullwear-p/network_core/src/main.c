/*
 * Project NULLWEAR — Network-core main entry point
 *
 * Copyright (c) 2026 Benjamin Jack Leighton, Tester Present Specialist
 * Automotive Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * The network core's job is small and singular: start the radio jammer
 * and stay out of the way. All real work happens in the RADIO_0_IRQHandler
 * and TIMER0_IRQHandler, which together implement the per-packet OUI
 * detection and CRC corruption pipeline.
 *
 * This main() exists only to:
 *   1. Initialise the IPC channel to the application core.
 *   2. Start the radio jammer.
 *   3. Sleep on WFI forever; the IRQs do everything else.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/ipc/ipc_service.h>
#include <zephyr/logging/log.h>

#include "radio_jammer.h"

LOG_MODULE_REGISTER(nullwear_net, LOG_LEVEL_INF);

/* IPC endpoint to the application core. Used to publish statistics
 * roughly once per second. */
static struct ipc_ept s_ep;

static void on_ept_bound(void *priv) {
    LOG_INF("IPC endpoint bound");
}

static void on_ept_recv(const void *data, size_t len, void *priv) {
    /* App core may request a stats snapshot. We respond synchronously. */
    if (len == 1 && *(const uint8_t *)data == 0x01) {
        nullwear_radio_stats_t stats;
        radio_jammer_get_stats(&stats);
        ipc_service_send(&s_ep, &stats, sizeof(stats));
    }
}

static struct ipc_ept_cfg s_ep_cfg = {
    .name = "nullwear_stats",
    .cb = {
        .bound = on_ept_bound,
        .received = on_ept_recv,
    },
};

int main(void)
{
    LOG_INF("NULLWEAR network-core firmware starting");

    /* Set up IPC to app core. */
    const struct device *ipc0 = DEVICE_DT_GET(DT_NODELABEL(ipc0));
    int err = ipc_service_open_instance(ipc0);
    if (err && err != -EALREADY) {
        LOG_ERR("ipc_service_open_instance failed: %d", err);
        return err;
    }
    err = ipc_service_register_endpoint(ipc0, &s_ep, &s_ep_cfg);
    if (err) {
        LOG_ERR("ipc_service_register_endpoint failed: %d", err);
        return err;
    }

    /* Start the radio. From this point on, the radio runs autonomously
     * via interrupt-driven hardware-event chains. */
    LOG_INF("Starting radio jammer; OUI target = 00:25:DF (Axon Enterprise)");
    radio_jammer_start();

    /* Hibernate. The IRQs will wake us only as needed. */
    while (1) {
        k_msleep(60000);  /* every 60s, we wake up but do nothing */
    }

    return 0;
}
