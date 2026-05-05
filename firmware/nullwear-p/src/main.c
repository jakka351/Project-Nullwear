/*
 * Project NULLWEAR — Application-core main
 *
 * Copyright (c) 2026 Benjamin Jack Leighton, Tester Present Specialist
 * Automotive Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * The application core handles everything that is NOT the radio:
 * battery management, LED status, button input, deep-sleep transitions,
 * and IPC consumption of the network core's running statistics.
 *
 * The radio jammer itself runs entirely on the network core and does
 * not depend on this firmware to operate. If the application core
 * crashes, the radio keeps jamming. This is by design.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/ipc/ipc_service.h>
#include <zephyr/logging/log.h>
#include "nullwear.h"

LOG_MODULE_REGISTER(nullwear_app, LOG_LEVEL_INF);

extern void battery_init(void);
extern void led_init(void);

/* IPC endpoint to network core. */
static struct ipc_ept s_ep_net;
static nullwear_stats_t s_last_stats;
static K_SEM_DEFINE(s_stats_ready, 0, 1);

static void on_net_bound(void *priv) {
    LOG_INF("Net-core IPC bound");
}

static void on_net_recv(const void *data, size_t len, void *priv) {
    if (len == sizeof(nullwear_stats_t)) {
        memcpy(&s_last_stats, data, sizeof(s_last_stats));
        k_sem_give(&s_stats_ready);
    }
}

static struct ipc_ept_cfg s_ep_cfg = {
    .name = "nullwear_stats",
    .cb = {
        .bound = on_net_bound,
        .received = on_net_recv,
    },
};

int main(void)
{
    LOG_INF("==================================");
    LOG_INF("  NULLWEAR/P firmware Rev A");
    LOG_INF("  Build: " __DATE__ " " __TIME__);
    LOG_INF("==================================");

    led_init();
    battery_init();

    const struct device *ipc0 = DEVICE_DT_GET(DT_NODELABEL(ipc0));
    int err = ipc_service_open_instance(ipc0);
    if (err && err != -EALREADY) {
        LOG_ERR("ipc_service_open_instance failed: %d", err);
        return err;
    }
    err = ipc_service_register_endpoint(ipc0, &s_ep_net, &s_ep_cfg);
    if (err) {
        LOG_ERR("ipc_service_register_endpoint failed: %d", err);
        return err;
    }

    /* Periodically poll the network core for stats. */
    while (1) {
        uint8_t cmd = 0x01;
        ipc_service_send(&s_ep_net, &cmd, 1);
        if (k_sem_take(&s_stats_ready, K_SECONDS(2)) == 0) {
            LOG_INF("Stats: rx=%u oui_match=%u jammed=%u hops=%u",
                s_last_stats.pkts_received_total,
                s_last_stats.pkts_oui_matched,
                s_last_stats.pkts_jammed,
                s_last_stats.channel_hops);
        }
        k_sleep(K_SECONDS(60));
    }
    return 0;
}
