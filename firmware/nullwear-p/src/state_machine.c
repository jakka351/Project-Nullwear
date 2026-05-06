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
 * Project NULLWEAR — App-core state machine
 *
 * Handles top-level lifecycle: boot, normal-operation, low-power-idle,
 * deep-sleep (USB-detached, battery present but officer not on shift).
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/pm/pm.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include "nullwear.h"

LOG_MODULE_REGISTER(state, LOG_LEVEL_INF);

typedef enum {
    ST_BOOT,
    ST_RUN,
    ST_IDLE,
    ST_DEEP_SLEEP,
    ST_FAULT,
} app_state_t;

static app_state_t s_state = ST_BOOT;

/* Tact switch — momentary press triggers a state report on USB-CDC
 * if connected; long-press initiates manual deep-sleep. */
static const struct gpio_dt_spec s_button =
    GPIO_DT_SPEC_GET(DT_NODELABEL(button0), gpios);
static struct gpio_callback s_btn_cb_data;
static int64_t s_btn_press_ms;

extern bat_state_t battery_state(void);

static void enter_state(app_state_t new_st)
{
    if (new_st == s_state) return;
    LOG_INF("State %d -> %d", s_state, new_st);
    s_state = new_st;
}

static void btn_isr(const struct device *p, struct gpio_callback *cb, uint32_t pins)
{
    int64_t now = k_uptime_get();
    int level = gpio_pin_get_dt(&s_button);
    if (level == 1) {
        s_btn_press_ms = now;
    } else {
        int64_t held = now - s_btn_press_ms;
        if (held > 3000) {
            LOG_INF("Long press — entering deep sleep");
            enter_state(ST_DEEP_SLEEP);
        } else {
            LOG_INF("Short press — status acknowledged");
            /* Future: emit a 1 Hz LED pulse as a presence cue. */
        }
    }
}

void state_machine_init(void)
{
    if (gpio_is_ready_dt(&s_button)) {
        gpio_pin_configure_dt(&s_button, GPIO_INPUT);
        gpio_pin_interrupt_configure_dt(&s_button, GPIO_INT_EDGE_BOTH);
        gpio_init_callback(&s_btn_cb_data, btn_isr, BIT(s_button.pin));
        gpio_add_callback(s_button.port, &s_btn_cb_data);
    }
    enter_state(ST_RUN);
}

void state_machine_thread(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

    state_machine_init();

    while (1) {
        bat_state_t bat = battery_state();

        switch (s_state) {
            case ST_RUN:
                if (bat == BAT_FAULT) enter_state(ST_FAULT);
                k_sleep(K_SECONDS(1));
                break;

            case ST_IDLE:
                /* Idle: radio still on but app core in low-power. */
                k_sleep(K_SECONDS(1));
                break;

            case ST_DEEP_SLEEP:
                LOG_INF("Entering deep sleep");
                /* Network core is left running — that is the entire point
                 * of the device. Only the app core enters low power. */
                pm_state_force(0u, &(struct pm_state_info){PM_STATE_SUSPEND_TO_RAM, 0, 0});
                k_sleep(K_FOREVER);
                break;

            case ST_FAULT:
                k_sleep(K_SECONDS(5));
                /* Try to recover. */
                if (bat != BAT_FAULT) enter_state(ST_RUN);
                break;

            default:
                k_sleep(K_SECONDS(1));
                break;
        }
    }
}

K_THREAD_DEFINE(state_tid, 2048, state_machine_thread,
    NULL, NULL, NULL, 10, 0, 1500);
