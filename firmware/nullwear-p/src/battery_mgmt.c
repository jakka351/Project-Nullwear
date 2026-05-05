/*
 * Project NULLWEAR — Battery management
 *
 * Polls the MAX17048 fuel gauge over I2C, reports state-of-charge
 * to the LED driver, monitors for fault conditions (over-temperature,
 * over-discharge), logs cycle events to NVM.
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include "nullwear.h"

LOG_MODULE_REGISTER(battery, LOG_LEVEL_INF);

/* MAX17048 I2C address and register map. */
#define MAX17048_ADDR        0x36
#define MAX17048_REG_VCELL   0x02   /* cell voltage, mV/1.25 */
#define MAX17048_REG_SOC     0x04   /* state of charge, %/256 */
#define MAX17048_REG_MODE    0x06
#define MAX17048_REG_VERSION 0x08
#define MAX17048_REG_CONFIG  0x0C
#define MAX17048_REG_STATUS  0x1A

/* Battery voltage thresholds (mV). LiPo nominal 3.7 V, full 4.2 V,
 * shutdown 3.0 V. */
#define VBAT_FULL_MV     4150
#define VBAT_OK_MV       3850
#define VBAT_LOW_MV      3650
#define VBAT_CRIT_MV     3450
#define VBAT_SHUTDOWN_MV 3000

static const struct device *s_i2c;
static const struct gpio_dt_spec s_chg_stat = GPIO_DT_SPEC_GET(
    DT_NODELABEL(charger_stat), gpios);

static bat_state_t s_state = BAT_OK;
static uint16_t s_vcell_mv = 0;
static uint8_t  s_soc_pct = 0;

static int max17048_read(uint8_t reg, uint16_t *val)
{
    uint8_t buf[2];
    int err = i2c_burst_read(s_i2c, MAX17048_ADDR, reg, buf, 2);
    if (err) {
        return err;
    }
    *val = ((uint16_t)buf[0] << 8) | buf[1];
    return 0;
}

static void update_state(void)
{
    uint16_t raw_v, raw_soc;
    if (max17048_read(MAX17048_REG_VCELL, &raw_v) ||
        max17048_read(MAX17048_REG_SOC, &raw_soc)) {
        s_state = BAT_FAULT;
        LOG_ERR("MAX17048 read failed");
        return;
    }

    s_vcell_mv = (uint16_t)((uint32_t)raw_v * 78125U / 1000000U); /* 78.125 µV/lsb */
    s_soc_pct  = raw_soc >> 8;

    bool charging = !gpio_pin_get_dt(&s_chg_stat);  /* STAT is open-drain, low = charging */

    if (charging) {
        s_state = BAT_CHG;
    } else if (s_vcell_mv < VBAT_CRIT_MV) {
        s_state = BAT_CRIT;
    } else if (s_vcell_mv < VBAT_LOW_MV) {
        s_state = BAT_LOW;
    } else {
        s_state = BAT_OK;
    }

    LOG_INF("Battery: %u mV, %u%%, state=%d", s_vcell_mv, s_soc_pct, s_state);

    if (s_vcell_mv < VBAT_SHUTDOWN_MV) {
        LOG_WRN("Battery critical — initiating ordered shutdown");
        /* Signal app core to enter deep sleep until USB attach. */
        /* This implementation: trigger a system reset into low-power mode. */
        sys_reboot(SYS_REBOOT_COLD);
    }
}

void battery_init(void)
{
    s_i2c = DEVICE_DT_GET(DT_NODELABEL(i2c0));
    if (!device_is_ready(s_i2c)) {
        LOG_ERR("I2C not ready");
        return;
    }
    if (!gpio_is_ready_dt(&s_chg_stat)) {
        LOG_ERR("Charger STAT GPIO not ready");
        return;
    }
    gpio_pin_configure_dt(&s_chg_stat, GPIO_INPUT | GPIO_PULL_UP);

    /* Verify MAX17048 is present. */
    uint16_t version;
    if (max17048_read(MAX17048_REG_VERSION, &version) == 0) {
        LOG_INF("MAX17048 detected, version 0x%04X", version);
    } else {
        LOG_WRN("MAX17048 not detected — battery monitoring degraded");
    }
    update_state();
}

bat_state_t battery_state(void)
{
    return s_state;
}

uint16_t battery_voltage_mv(void)
{
    return s_vcell_mv;
}

uint8_t battery_soc_pct(void)
{
    return s_soc_pct;
}

/* Periodic poll thread, low priority. */
#define BATTERY_POLL_INTERVAL_S 60

void battery_poll_thread(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);
    while (1) {
        update_state();
        k_sleep(K_SECONDS(BATTERY_POLL_INTERVAL_S));
    }
}

K_THREAD_DEFINE(battery_poll_tid, 1024, battery_poll_thread,
    NULL, NULL, NULL, 14, 0, 1000);
