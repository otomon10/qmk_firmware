/* Copyright 2020 sekigon-gonnoc
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
#include "pointing_device.h"
#include "keymap_def.h"
#include "paw3204.h"
#include "trackball.h"
#include "./keymaps/tps61291.h"
#include "./keymaps/common.h"

#define ENABLE_HOLD_TIME 200
#define MOUSE_SPEED_SHIFT_DEFAULT 0.8

// Defines the keycodes used by our macros in process_record_user
enum custom_keycodes {
    CK_START = BMP_SAFE_RANGE,
    CK_LTFN,
    CK_MOVL,
    CK_MOVR,
    CK_TAG,
    CK_UNTAG,
    CK_LBTN,
    CK_RBTN,
    CK_CBTN,
    CK_END,
};

const key_string_map_t custom_keys_user = {
    .start_kc = CK_START,
    .end_kc   = CK_END,
    .key_strings =
        "CK_START\0CK_LTFN\0CK_MOVL\0CK_MOVR\0CK_TAG\0CK_UNTAG\0CK_LBTN\0CK_"
        "RBTN\0CK_CBTN\0CK_END\0"};

const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS] = {{{KC_NO}}};

bool     is_shitft_pressed;
bool     tg_aesc_enable;
bool     is_cspace_fn_active   = false;
uint16_t cspace_fn_timer       = 0;
bool     is_my_tag_active      = false;
uint16_t my_tag_timer          = 0;
bool     is_my_untag_ms_active = false;
uint16_t my_untag_ms_timer     = 0;

uint32_t keymaps_len() { return sizeof(keymaps) / sizeof(uint16_t); }

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    bool continue_process = process_record_user_bmp(keycode, record);
    if (continue_process == false) {
        return false;
    }

    switch (keycode) {
        case KC_RSHIFT:
        case KC_LSHIFT: {
            if (record->event.pressed) {
                is_shitft_pressed = true;
            } else {
                is_shitft_pressed = false;
                if (tg_aesc_enable) {
                    unregister_code(KC_RALT);
                    tg_aesc_enable = false;
                }
            }
            break;
        }
        case CK_LTFN: {
            if (record->event.pressed) {
                is_cspace_fn_active = true;
                cspace_fn_timer     = timer_read();
            } else {
                if (timer_elapsed(cspace_fn_timer) <= ENABLE_HOLD_TIME) {
                    register_code(KC_LCTRL);
                    register_code(KC_SPACE);
                    unregister_code(KC_LCTRL);
                    unregister_code(KC_SPACE);
                }
                layer_off(_FN_MISC);
                is_cspace_fn_active = false;
            }
            return false;
            break;
        }
        case CK_MOVL:
        case CK_MOVR: {
            if (record->event.pressed) {
                if (is_shitft_pressed) {
                    if (!tg_aesc_enable) {
                        // release when the shift key is released
                        register_code(KC_RALT);
                    }

                    // toggle window
                    if (keycode == CK_MOVL) {
                        unregister_code(KC_LSHIFT);
                    }
                    register_code(KC_ESC);
                    unregister_code(KC_ESC);
                    if (keycode == CK_MOVL) {
                        register_code(KC_LSHIFT);
                    }

                    tg_aesc_enable = true;
                } else {
                    if (keycode == CK_MOVL) {
                        register_code(KC_LSHIFT);
                    }
                    register_code(KC_RCTRL);
                    register_code(KC_TAB);
                    if (keycode == CK_MOVL) {
                        unregister_code(KC_LSHIFT);
                    }
                    unregister_code(KC_RCTRL);
                    unregister_code(KC_TAB);
                }
            }
            return false;
        }
        case CK_TAG: {
            if (record->event.pressed) {
                is_my_tag_active = true;
                my_tag_timer     = timer_read();
            } else {
                if (timer_elapsed(my_tag_timer) <= ENABLE_HOLD_TIME) {
                    register_code(KC_F12);
                    unregister_code(KC_F12);
                }
                layer_off(_FN_MISC);
                is_my_tag_active = false;
            }
            return false;
        }
        case CK_UNTAG: {
            if (record->event.pressed) {
                is_my_untag_ms_active = true;
                my_untag_ms_timer     = timer_read();
            } else {
                if (timer_elapsed(my_untag_ms_timer) <= ENABLE_HOLD_TIME) {
                    register_code(KC_LALT);
                    register_code(KC_LCTRL);
                    register_code(KC_MINS);
                    unregister_code(KC_LALT);
                    unregister_code(KC_LCTRL);
                    unregister_code(KC_MINS);
                }
                layer_off(_MOUSE);
                is_my_untag_ms_active = false;
            }
            return false;
        }
        case CK_LBTN:
        case CK_RBTN:
        case CK_CBTN: {
            // click, drag & drop
            report_mouse_t report = pointing_device_get_report();
            uint8_t        btn    = 1 << (keycode - CK_LBTN);
            if (record->event.pressed) {
                report.buttons |= btn;
            } else {
                report.buttons &= ~btn;
            }
            pointing_device_set_report(report);
            return false;
        }
        default:
            break;
    }

    return true;
}

uint32_t layer_state_set_user(uint32_t state) {
    switch (biton32(state)) {
        case _BASE:
            rgblight_sethsv_noeeprom(HSV_OFF);
            break;
        case _NUMS:
            rgblight_sethsv_noeeprom(HSV_GOLD);
            break;
        case _MOUSE:
            rgblight_sethsv_noeeprom(HSV_TEAL);
            break;
        case _FN_MISC:
            rgblight_sethsv_noeeprom(HSV_CHARTREUSE);
            break;
    }
    return state;
}

void matrix_init_user(void) {
    /* init trackball */
    init_paw3204();
    /* init touch sensor */
    init_ttp223();
    /* init voltage boost converter */
    init_tps61291();
}

void matrix_scan_user(void) {
    static int  cnt           = 0;

    battery_task();
    init_led_task(cnt);

    if (is_cspace_fn_active) {
        if (timer_elapsed(cspace_fn_timer) > ENABLE_HOLD_TIME) {
            layer_on(_FN_MISC);
            is_cspace_fn_active = false;
        }
    }
    if (is_my_tag_active) {
        if (timer_elapsed(my_tag_timer) > ENABLE_HOLD_TIME) {
            layer_on(_FN_MISC);
            is_my_tag_active = false;
        }
    }
    if (is_my_untag_ms_active) {
        if (timer_elapsed(my_untag_ms_timer) > ENABLE_HOLD_TIME) {
            layer_on(_MOUSE);
            is_my_untag_ms_active = false;
        }
    }

    process_trackball(MOUSE_SPEED_SHIFT_DEFAULT);

    cnt++;
}

void led_set_user(uint8_t usb_led) {}
