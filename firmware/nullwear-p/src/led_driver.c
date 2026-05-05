/*
 * Project NULLWEAR — RGB LED driver
 *
 * SPDX-License-Identifier: MIT
 *
 * Drives an 0805 common-anode RGB LED indicating battery state and
 * (optionally) recent jam-pulse activity. PWM-driven for dimming.
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>
#include "nullwear.h"

LOG_MODULE_REGISTER(led, LOG_LEVEL_INF);

extern bat_state_t battery_state(void);

static const struct pwm_dt_spec s_red   = PWM_DT_SPEC_GET(DT_NODELABEL(led_r));
static const struct pwm_dt_spec s_green = PWM_DT_SPEC_GET(DT_NODELABEL(led_g));
static const struct pwm_dt_spec s_blue  = PWM_DT_SPEC_GET(DT_NODELABEL(led_b));

#define PWM_PERIOD_NS  (1000000U)   /* 1 ms = 1 kHz */

static void set_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    pwm_set_dt(&s_red,   PWM_PERIOD_NS, (PWM_PERIOD_NS / 255U) * r);
    pwm_set_dt(&s_green, PWM_PERIOD_NS, (PWM_PERIOD_NS / 255U) * g);
    pwm_set_dt(&s_blue,  PWM_PERIOD_NS, (PWM_PERIOD_NS / 255U) * b);
}

void led_init(void)
{
    /* Quick hello: cycle through R/G/B once to confirm wiring. */
    set_rgb(64, 0, 0); k_sleep(K_MSEC(150));
    set_rgb(0, 64, 0); k_sleep(K_MSEC(150));
    set_rgb(0, 0, 64); k_sleep(K_MSEC(150));
    set_rgb(0, 0, 0);
}

void led_thread(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);
    bool blink_phase = false;
    while (1) {
        bat_state_t st = battery_state();
        switch (st) {
            case BAT_OK:
                set_rgb(0, 24, 0);  /* dim green */
                k_sleep(K_MSEC(2000));
                break;
            case BAT_LOW:
                set_rgb(40, 24, 0); /* amber */
                k_sleep(K_MSEC(2000));
                break;
            case BAT_CRIT:
                set_rgb(blink_phase ? 80 : 0, 0, 0); /* red blink */
                blink_phase = !blink_phase;
                k_sleep(K_MSEC(500));
                break;
            case BAT_CHG:
                /* Pulsing green: slow breath. */
                for (int i = 0; i < 80; i += 4) { set_rgb(0, i, 0); k_sleep(K_MSEC(40)); }
                for (int i = 80; i > 0; i -= 4) { set_rgb(0, i, 0); k_sleep(K_MSEC(40)); }
                break;
            case BAT_FAULT:
                set_rgb(blink_phase ? 100 : 0, 0, 0);
                blink_phase = !blink_phase;
                k_sleep(K_MSEC(150));
                break;
        }
    }
}

K_THREAD_DEFINE(led_tid, 1024, led_thread,
    NULL, NULL, NULL, 12, 0, 500);
