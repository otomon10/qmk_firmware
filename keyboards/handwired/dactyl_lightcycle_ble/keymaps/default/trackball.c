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

/* mouse define */
#define MOVE_CNT_MAX 100
#define MOVE_CNT_THRESH 10
#define m 3.14159265

uint8_t g_move_cnt;
uint8_t g_stop_cnt;
bool g_force_move;
bool g_force_stop;

bool is_ready_trackball(void) {
    uint8_t pid = read_pid_paw3204();
    return pid == 0x30 ? true : false;
}

void send_trackball_report(const int mouse_speed) {
    uint8_t stat;
    int8_t x, y;
    report_mouse_t mouse_rep;
    double rad = 45 * (PI / 180);
    bool moved;

    read_paw3204(&stat, &x, &y);
    // dprintf("stat:%3d x:%3d y:%3d\n", stat, x, y);

    mouse_rep = pointing_device_get_report();
    mouse_rep.x = (int16_t)(-x * cos(rad) - y * sin(rad)) >> mouse_speed;
    mouse_rep.y = (int16_t)(x * sin(rad) - y * cos(rad)) >> mouse_speed;

    /* Removed the initialization process of pointing_device_send(), */
    /* which should send the mouse report even when it is stopped. */
    pointing_device_set_report(mouse_rep);

    moved = (stat & 0x80) ? true : false;

    if (moved) {
        if (g_move_cnt < MOVE_CNT_MAX) g_move_cnt++;
        if (g_move_cnt > MOVE_CNT_THRESH) {
            g_stop_cnt = 0;
        }
    } else {
        g_move_cnt = 0;
        if (g_stop_cnt < MOVE_CNT_MAX) g_stop_cnt++;
    }
}

bool is_trackball_stop(void) {
    return g_stop_cnt == MOVE_CNT_MAX ? true : false;
}

void process_trackball(const int mouse_speed) {
    static int cnt;
    static bool paw_ready;

    if (cnt++ % 50 == 0) {
        paw_ready = is_ready_trackball();
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
