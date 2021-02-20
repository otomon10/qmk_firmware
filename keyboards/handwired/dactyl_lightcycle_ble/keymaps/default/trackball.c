/*
Copyright 2020 otomon10

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "trackball.h"

#include <math.h>

#include "app_ble_func.h"
#include "keymap_def.h"
#include "paw3204.h"
#include "pointing_device.h"

/* nrf52 */
#if defined PROTOCOL_NRF
#include "nrf_gpio.h"
#include "pin_assign.h"
#define readPin(pin) nrf_gpio_pin_read(pin)
#define setPinInputHigh(pin) nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_PULLUP);
#endif

/* mouse define */
#define MOVE_CNT_MAX 10
#define MOVE_CNT_THRESH 5
#define PI 3.14159265

/* touch sensor */
#define TTP223_PIN D2

uint8_t g_move_cnt;
uint8_t g_stop_cnt;
bool g_force_move;
bool g_force_stop;
bool g_touch_detect = false;

void init_ttp223() { setPinInputHigh(TTP223_PIN); }

bool is_touched() { return readPin(TTP223_PIN) == 1; }

bool is_ready_trackball(void) {
    uint8_t pid = read_pid_paw3204();
    return pid == 0x30 ? true : false;
}

void calculate_mouse_rep(const int8_t x, const int8_t y,
                         report_mouse_t *mouse_rep, const double mouse_speed) {
    const double rad = 45 * (PI / 180);
    double tmp;

    tmp = (-x * cos(rad) - y * sin(rad)) * mouse_speed;
    if (tmp > 45 || tmp < -45) {
        tmp = tmp * 4.0;
    } else if (tmp > 25 || tmp < -25) {
        tmp = tmp * 2.0;
    }
    if (tmp > 127) {
        tmp = 127;
    } else if (tmp < -128) {
        tmp = -128;
    }
    mouse_rep->x = (int8_t)tmp;

    tmp = (x * sin(rad) - y * cos(rad)) * mouse_speed;
    if (tmp > 45 || tmp < -45) {
        tmp = tmp * 4.0;
    } else if (tmp > 25 || tmp < -25) {
        tmp = tmp * 2.0;
    }
    if (tmp > 127) {
        tmp = 127;
    } else if (tmp < -128) {
        tmp = -128;
    }
    mouse_rep->y = (int8_t)tmp;
}

void send_trackball_report(const double mouse_speed) {
    uint8_t stat;
    int8_t x, y;
    report_mouse_t mouse_rep;
    bool moved;
    static bool send_stop = false;

    read_paw3204(&stat, &x, &y);
    mouse_rep = pointing_device_get_report();

    calculate_mouse_rep(x, y, &mouse_rep, mouse_speed);

    moved = (stat & 0x80) ? true : false;
    if (moved) {
#if 0
        dprintf("stat:%3d x:%3d y:%3d mx:%3d my:%3d\n", stat, x, y, mouse_rep.x,
                mouse_rep.y);
#endif
        pointing_device_set_report(mouse_rep);
        send_stop = false;
        if (g_move_cnt < MOVE_CNT_MAX) g_move_cnt++;
        if (g_move_cnt > MOVE_CNT_THRESH) {
            g_stop_cnt = 0;
        }
    } else {
        if (!send_stop) {
            pointing_device_set_report(mouse_rep);
            send_stop = true;
        }
        g_move_cnt = 0;
        if (g_stop_cnt < MOVE_CNT_MAX) g_stop_cnt++;
    }
}

bool is_trackball_stop(void) {
    return g_stop_cnt == MOVE_CNT_MAX ? true : false;
}

void process_trackball(const double mouse_speed) {
    static int cnt;
    static bool paw_ready;

    if (cnt++ % 50 == 0) {
        paw_ready = is_ready_trackball();
    }

    if (g_touch_detect) {
        if (is_touched()) {
            enable_trackball_force_move();
        } else {
            disable_trackball_force_move_with_delay();
        }
    }

    if (paw_ready) {
        static bool stop_prev;
        bool stop_now;

        send_trackball_report(mouse_speed);

        stop_now = is_trackball_stop();

        if (g_force_move) {
            stop_now = false;
        }

        if (g_force_stop) {
            stop_now = true;
            g_stop_cnt = MOVE_CNT_MAX;
            g_force_stop = false;
        }

        if (stop_now != stop_prev) {
            if (stop_now) {
                layer_off(_MOUSE);
            } else {
                layer_on(_MOUSE);
            }
            stop_prev = stop_now;
        }
    }

    cnt++;
}

void enable_trackball_force_move() { g_force_move = true; }

void disable_trackball_force_move() {
    g_force_move = false;
    g_force_stop = true;
}

void disable_trackball_force_move_with_delay() { g_force_move = false; }

void trackball_continue_moving() { g_stop_cnt = 0; }

void enable_touch_detect() { g_touch_detect = true; }
void disable_touch_detect() { g_touch_detect = false; }
void toggle_touch_detect() { g_touch_detect = !g_touch_detect; }
