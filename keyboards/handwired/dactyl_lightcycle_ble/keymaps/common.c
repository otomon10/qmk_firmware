/*
Copyright 2021 otomon10

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

#include "common.h"
#include "quantum.h"
#include "./keymaps/tps61291.h"

void init_led_task(int count) {
    static bool led_initialized = false;

    /* init rgblight did not work well in keyboard_post_init_user(), so do it
     * here */
    if (!led_initialized) {
        if (count <= 30) {
            int voltage = get_non_boost_voltage();
            if (voltage == 0) {
                return;
            }
            if (voltage < 2250) {
                rgblight_sethsv_noeeprom(HSV_RED);
            } else if (voltage < 2390) {
                rgblight_sethsv_noeeprom(HSV_YELLOW);
            } else {
                rgblight_sethsv_noeeprom(HSV_WHITE);
            }
        } else if (count > 30) {
            rgblight_sethsv_noeeprom(HSV_OFF);
            led_initialized = true;
        }
    }
}

void bmp_before_sleep() { rgblight_sethsv_noeeprom(HSV_OFF); }
