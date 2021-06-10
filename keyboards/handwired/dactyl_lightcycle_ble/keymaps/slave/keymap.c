/* Copyright 2019 sekigon-gonnoc
 * COpyright 2021 otomon10
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include QMK_KEYBOARD_H
#include "bmp.h"
#include "bmp_custom_keycode.h"
#include "keycode_str_converter.h"
#include "./keymaps/tps61291.h"

// Defines the keycodes used by our macros in process_record_user
enum custom_keycodes {
    CUSTOM_KEYCODE_START = BMP_SAFE_RANGE,
};

const key_string_map_t custom_keys_user = {.start_kc    = CUSTOM_KEYCODE_START,
                                           .end_kc      = CUSTOM_KEYCODE_START,
                                           .key_strings = "\0"};

const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS] = {{{KC_NO}}};

uint32_t keymaps_len() { return sizeof(keymaps) / sizeof(uint16_t); }

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    bool continue_process = process_record_user_bmp(keycode, record);
    if (continue_process == false) {
        return false;
    }

    return true;
}

void matrix_init_user(void) {
    /* init voltage boost converter */
    init_tps61291();
}

void matrix_scan_user(void) {
    static int  cnt           = 0;
    static bool init_rgblight = false;

    /* If the slave has a USB connection, it will be forced into dfu mode */
    if (cnt < 100 && is_usb_connected()) {
        BMPAPI->bootloader_jump();
    }

    /* init rgblight did not work well in keyboard_post_init_user(), so do it
     * here */
    if (!init_rgblight) {
        if (cnt == 0) {
            rgblight_sethsv_noeeprom(HSV_WHITE);
        } else if (cnt > 30) {
            rgblight_sethsv_noeeprom(HSV_OFF);
            init_rgblight = true;
        }
    }

    battery_task();

    cnt++;
}

void led_set_user(uint8_t usb_led) {}
