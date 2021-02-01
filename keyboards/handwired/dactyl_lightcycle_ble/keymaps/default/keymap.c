/*
Copyright 2018 Sekigon
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

#include QMK_KEYBOARD_H
#include "adc.h"
#include "app_ble_func.h"
#include "ble_helper.h"
#include "keymap_def.h"
#include "pointing_device.h"
#include "tps61291.h"
#include "trackball.h"

/* Aliases */
#define C(kc) LCTL(kc)
#define S(kc) LSFT(kc)
#define A(kc) LALT(kc)
#define G(kc) LGUI(kc)

/* dynamic macro define */
#define KC_DRS1 DYN_REC_START1
#define KC_DRS2 DYN_REC_START2
#define KC_DMP1 DYN_MACRO_PLAY1
#define KC_DMP2 DYN_MACRO_PLAY2
#define KC_DRS DYN_REC_STOP

/* mouse define */
#define MOUSE_CNT_MAX 100
#define MOUSE_MOVE_CNT 10
#define MOUSE_SPEED_SHIFT_DEFAULT 0.8
#define MOUSE_SPEED_SHIFT_SLOW 0.3
#define MS_WH_THRESHOLD_DEFAULT 4
#define MS_WH_SPEEDUP_INTERVAL 20

#define BLE_SLEEP_TITME (3600 * 30) /* sleep in 30 min */

/* Alt + ESC */
#define AESC_ENABLE_TIME 30

/* layer change hold time */
#define ENABLE_HOLD_TIME 200

enum custom_keycodes {
    /* ble micro pro */
    AD_WO_L = SAFE_RANGE, /* Start advertising without whitelist  */
    BLE_DIS,              /* Disable BLE HID sending              */
    BLE_EN,               /* Enable BLE HID sending               */
    USB_DIS,              /* Disable USB HID sending              */
    USB_EN,               /* Enable USB HID sending               */
    DELBNDS,              /* Delete all bonding                   */
    ADV_ID0,              /* Start advertising to PeerID 0        */
    ADV_ID1,              /* Start advertising to PeerID 1        */
    ADV_ID2,              /* Start advertising to PeerID 2        */
    ADV_ID3,              /* Start advertising to PeerID 3        */
    ADV_ID4,              /* Start advertising to PeerID 4        */
    BATT_LV,              /* Display battery level in milli volts */
    DEL_ID0,              /* Delete bonding of PeerID 0           */
    DEL_ID1,              /* Delete bonding of PeerID 1           */
    DEL_ID2,              /* Delete bonding of PeerID 2           */
    DEL_ID3,              /* Delete bonding of PeerID 3           */
    DEL_ID4,              /* Delete bonding of PeerID 4           */
    ENT_DFU,              /* Start bootloader                     */
    ENT_SLP,              /* Deep sleep mode                      */
    /* custom */
    MS_LBTN,
    MS_RBTN,
    MS_CBTN,
    MS_SLOW,
    MY_MOVR,
    MY_MOVL,
    MY_TAG,
    MY_SPACE_FN,
    MY_UNTAG_MS,
    RGB_BASE,
    /* always put DYNAMIC_MACRO_RANGE last */
    DYNAMIC_MACRO_RANGE,
};
#include "dynamic_macro.h" /* include after DYNAMIC_MACRO_RANGE definition */

enum tap_dances { CT_CTRL = 0, CT_ALT };

void dance_ctrl_finished(qk_tap_dance_state_t *state, void *user_data) {
    if (state->count == 1) {
        register_code(KC_LCTRL);
    } else {
        register_code(KC_LCTRL);
        register_code(KC_LSHIFT);
    }
}

void dance_ctrl_reset(qk_tap_dance_state_t *state, void *user_data) {
    if (state->count == 1) {
        unregister_code(KC_LCTRL);
    } else {
        unregister_code(KC_LCTRL);
        unregister_code(KC_LSHIFT);
    }
}

void dance_alt_finished(qk_tap_dance_state_t *state, void *user_data) {
    if (state->count == 1) {
        register_code(KC_LALT);
    } else if (state->count == 2) {
        register_code(KC_LALT);
        register_code(KC_LCTRL);
    } else {
        register_code(KC_LALT);
        register_code(KC_LCTRL);
        register_code(KC_LSHIFT);
    }
}

void dance_alt_reset(qk_tap_dance_state_t *state, void *user_data) {
    if (state->count == 1) {
        unregister_code(KC_LALT);
    } else if (state->count == 2) {
        unregister_code(KC_LALT);
        unregister_code(KC_LCTRL);
    } else {
        unregister_code(KC_LALT);
        unregister_code(KC_LCTRL);
        unregister_code(KC_LSHIFT);
    }
}

qk_tap_dance_action_t tap_dance_actions[] = {
    [CT_CTRL] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, dance_ctrl_finished,
                                             dance_ctrl_reset),
    [CT_ALT] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, dance_alt_finished,
                                            dance_alt_reset)};

/* global variables */
bool is_shitft_pressed;
double mouse_speed = MOUSE_SPEED_SHIFT_DEFAULT;
bool tg_aesc_enable;
bool is_cspace_fn_active = false;
uint16_t cspace_fn_timer = 0;
bool is_my_tag_active = false;
uint16_t my_tag_timer = 0;
bool is_my_untag_ms_active = false;
uint16_t my_untag_ms_timer = 0;
bool enable_rgb_base = false;
int sleep_cnt = 0;

// clang-format off
const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
[_BASE] = LAYOUT( \
	// Left
	KC_ESC,			KC_Q,		KC_W,		KC_E,		KC_R,		KC_T,					XXXXXXX,	XXXXXXX,		\
	KC_TAB,			KC_A,		KC_S,		KC_D,		KC_F,		KC_G,					KC_BSPACE,	MY_SPACE_FN,	\
	KC_LSHIFT,		KC_Z,		KC_X,		KC_C,		KC_V,		KC_B,					XXXXXXX,	TD(CT_ALT),		\
	RALT(KC_F7),	KC_LGUI,	KC_GRV,		MY_MOVL,	MY_MOVR,				KC_SPACE,	KC_LSHIFT,	TD(CT_CTRL),	\
	// Right
	XXXXXXX,		XXXXXXX,				KC_Y,		KC_U,		KC_I,		KC_O,		KC_P,		KC_BSLASH,		\
	MY_TAG,			KC_DELETE,				KC_H,		KC_J,		KC_K,		KC_L,		KC_SCOLON,	KC_QUOTE,		\
	MY_UNTAG_MS,	XXXXXXX,				KC_N,		KC_M,		KC_COMMA,	KC_DOT,		KC_SLASH,	KC_RSHIFT,		\
	XXXXXXX,		MO(_NUMS),	KC_ENTER,				KC_LEFT,	KC_DOWN,	KC_UP,		KC_RIGHT,	RALT(KC_F8)		\
),

[_NUMS] = LAYOUT( \
	// Left
	_______,	_______,	_______,	_______,	_______,	_______,				XXXXXXX,	XXXXXXX,	\
	_______,	KC_1,		KC_2,		KC_3,		KC_4,		KC_5,					_______,	_______,	\
	_______,	_______,	_______,	_______,	_______,	_______,				XXXXXXX,	_______,	\
	_______,	_______,	_______,	_______,	_______,				_______,	_______,	_______,	\
	// Right
	XXXXXXX,	XXXXXXX,				_______,	_______,	_______,	_______,	_______,	_______,	\
	_______,	_______,				KC_6,		KC_7,		KC_8,		KC_9,		KC_0,		_______,	\
	_______,	XXXXXXX,				_______,	KC_MINS,	KC_EQL,		KC_LBRC,	KC_RBRC,	_______,	\
	XXXXXXX,	_______,	_______,				KC_HOME,	KC_PGDN,	KC_PGUP,	KC_END,		_______		\
),

[_MOUSE] = LAYOUT( \
	// Left
	_______,	_______,	_______,	_______,	_______,	_______,				XXXXXXX,	XXXXXXX,	\
	_______,	_______,	_______,	_______,	_______,	_______,				_______,	_______,	\
	_______,	_______,	_______,	_______,	_______,	_______,				XXXXXXX,	_______,	\
	_______,	_______,	_______,	_______,	_______,				_______,	_______,	_______,	\
	// Right
	XXXXXXX,	XXXXXXX,				KC_WH_L,		MS_CBTN,	KC_WH_R,	_______,	_______,	_______,	\
	_______,	_______,				C(S(KC_TAB)),	KC_WH_U,	C(KC_TAB),	_______,	_______,	_______,	\
	_______,	XXXXXXX,				MS_LBTN,		KC_WH_D,	MS_RBTN,	MS_SLOW,	_______,	_______,	\
	XXXXXXX,	_______,	_______,					_______,	_______,	_______,	_______,	_______		\
),

[_FN_MISC] = LAYOUT( \
	// Left
	AD_WO_L,	ADV_ID0,	ADV_ID1,	 ADV_ID2,	ADV_ID3,	ADV_ID4,				XXXXXXX,	XXXXXXX,	\
	KC_F11,		KC_F1,		KC_F2,  	KC_F3,		KC_F4,		KC_F5,					KC_DRS2,	_______,	\
	RGB_TOG,	_______ ,	_______,	_______,	_______,	_______,				XXXXXXX,	KC_DRS1,	\
	RGB_BASE,	KC_LEFT,	KC_DOWN,	KC_UP,		KC_RIGHT,				KC_ENTER,	_______,	KC_DRS,		\
	// Right
	XXXXXXX,	XXXXXXX,				BLE_DIS,	BLE_EN,		USB_DIS,	USB_EN,		BATT_LV,	DELBNDS,	\
	_______,	KC_DMP2,				KC_F6,		KC_F7,		KC_F8,		KC_F9,		KC_F10,		KC_F12,		\
	KC_DMP1,	XXXXXXX,				_______,	_______,	_______,	_______,	_______,	_______,	\
	XXXXXXX,	_______,	_______,				KC_PSCR,	KC_INS,		_______,	AD_WO_L,	ENT_DFU 	\
),

[_MISC2] = LAYOUT( \
	// Left
	_______,	_______,	_______,	_______,	_______,	_______,				XXXXXXX,	XXXXXXX,	\
	_______,	_______,	_______,	_______,	_______,	_______,				_______,	_______,	\
	_______,	_______,	_______,	_______,	_______,	_______,				XXXXXXX,	_______,	\
	_______,	_______,	_______,	_______,	_______,				_______,	_______,	_______,	\
	// Right
	XXXXXXX,	XXXXXXX,				KC_NLCK,	KC_P7,		KC_P8,		KC_P9,		KC_PMNS,	_______,	\
	_______,	_______,				KC_PSLS,	KC_P4,		KC_P5,		KC_P6,		KC_PPLS,	_______,	\
	_______,	XXXXXXX,				KC_PAST,	KC_P1,		KC_P2,		KC_P3,		KC_PENT,	_______,	\
	XXXXXXX,	_______,	_______,				KC_P0,		KC_P0,		KC_PDOT,	KC_PENT,	_______		\
),
};
// clang-format on

bool process_record_user_ble(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        switch (keycode) {
            case DELBNDS:
                delete_bonds();
                return false;
            case AD_WO_L:
                ble_connect_advertising_wo_whitelist();
                return false;
            case USB_EN:
                set_usb_enabled(true);
                return false;
                break;
            case USB_DIS:
                set_usb_enabled(false);
                return false;
                break;
            case BLE_EN:
                set_ble_enabled(true);
                return false;
                break;
            case BLE_DIS:
                set_ble_enabled(false);
                return false;
                break;
            case ADV_ID0:
            case ADV_ID1:
            case ADV_ID2:
            case ADV_ID3:
            case ADV_ID4: {
                uint8_t id = keycode - ADV_ID0;
                ble_connect_id(id);
                return false;
            }
            case DEL_ID0:
                delete_bond_id(0);
                return false;
            case DEL_ID1:
                delete_bond_id(1);
                return false;
            case DEL_ID2:
                delete_bond_id(2);
                return false;
            case DEL_ID3:
                delete_bond_id(3);
                return false;
            case DEL_ID4:
                delete_bond_id(4);
                return false;
            case BATT_LV: {
                char str[16];
                sprintf(str, "%4dmV", get_non_boost_voltage());
                send_string(str);
                return false;
            }
            case ENT_DFU:
                rgblight_sethsv_noeeprom(302, 255, 255);
                nrf_delay_ms(10);
                bootloader_jump();
                return false;
        }
    } else if (!record->event.pressed) {
        switch (keycode) {
            case ENT_SLP:
                sleep_mode_enter();
                return false;
        }
    }
    return true;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    sleep_cnt = 0;

    /* dynamic macros */
    if (!process_record_dynamic_macro(keycode, record)) {
        return false;
    }

    /* FIXME: In BLE pro micro, if there are different keycodes after a layer
     * change, the released event may not occur. */

    /* ble */
    bool ret = true;
    ret = process_record_user_ble(keycode, record);

    /* custom */
    switch (keycode) {
        case KC_WH_U:
        case KC_WH_D:
        case KC_WH_L:
        case KC_WH_R: {
            if (record->event.pressed) {
                // enable_trackball_force_move();
            } else {
                // disable_trackball_force_move_with_delay();
            }
            // trackball_continue_moving();
            break;
        }
        case MS_LBTN:
        case MS_RBTN:
        case MS_CBTN: {
            // click, drag & drop
            report_mouse_t report = pointing_device_get_report();
            uint8_t btn = 1 << (keycode - MS_LBTN);
            if (record->event.pressed) {
                report.buttons |= btn;
                // enable_trackball_force_move();
            } else {
                report.buttons &= ~btn;
                // disable_trackball_force_move_with_delay();
            }
            pointing_device_set_report(report);
            // trackball_continue_moving();
            return false;
        }
        case MS_SLOW: {
            if (record->event.pressed) {
                mouse_speed = mouse_speed == MOUSE_SPEED_SHIFT_DEFAULT
                                  ? MOUSE_SPEED_SHIFT_SLOW
                                  : MOUSE_SPEED_SHIFT_DEFAULT;
            }
            return false;
        }
        case MY_MOVL:
        case MY_MOVR: {
            if (record->event.pressed) {
                if (is_shitft_pressed) {
                    if (!tg_aesc_enable) {
                        // release when the shift key is released
                        register_code(KC_RALT);
                    }

                    // toggle window
                    if (keycode == MY_MOVL) {
                        unregister_code(KC_LSHIFT);
                    }
                    register_code(KC_ESC);
                    unregister_code(KC_ESC);
                    if (keycode == MY_MOVL) {
                        register_code(KC_LSHIFT);
                    }

                    tg_aesc_enable = true;
                } else {
                    if (keycode == MY_MOVL) {
                        register_code(KC_LSHIFT);
                    }
                    register_code(KC_RCTRL);
                    register_code(KC_TAB);
                    if (keycode == MY_MOVL) {
                        unregister_code(KC_LSHIFT);
                    }
                    unregister_code(KC_RCTRL);
                    unregister_code(KC_TAB);
                }
            }
            return false;
        }
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
        case MY_TAG: {
            if (record->event.pressed) {
                is_my_tag_active = true;
                my_tag_timer = timer_read();
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
        case MY_SPACE_FN: {
            if (record->event.pressed) {
                is_cspace_fn_active = true;
                cspace_fn_timer = timer_read();
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
        }
        case MY_UNTAG_MS: {
            if (record->event.pressed) {
                is_my_untag_ms_active = true;
                my_untag_ms_timer = timer_read();
            } else {
                if (timer_elapsed(my_untag_ms_timer) <= ENABLE_HOLD_TIME) {
                    register_code(KC_LALT);
                    register_code(KC_LCTRL);
                    register_code(KC_MINS);
                    unregister_code(KC_LALT);
                    unregister_code(KC_LCTRL);
                    unregister_code(KC_MINS);
                }
                layer_off(_MISC2);
                is_my_untag_ms_active = false;
            }
            return false;
        }
        case RGB_BASE: {
            if (record->event.pressed) {
                enable_rgb_base = enable_rgb_base ? false : true;
            }
            return false;
        }
    }

    return ret;
}

uint32_t layer_state_set_user(uint32_t state) {
    switch (biton32(state)) {
        case _BASE:
            if (enable_rgb_base) {
                rgblight_sethsv_noeeprom(HSV_WHITE);
            } else {
                rgblight_sethsv_noeeprom(HSV_OFF);
            }
            break;
        case _NUMS:
            rgblight_sethsv_noeeprom(HSV_OBLIVION_YELLOW);
            break;
        case _MOUSE:
            rgblight_sethsv_noeeprom(HSV_OBLIVION_ORANGE);
            break;
        case _FN_MISC:
            rgblight_sethsv_noeeprom(HSV_OBLIVION_GREEN);
            break;
        case _MISC2:
            rgblight_sethsv_noeeprom(HSV_OBLIVION_BLUE);
            break;
    }
    return state;
}

void keyboard_post_init_user(void) {
    // debug_enable=true;
    // debug_matrix=true;
    // debug_keyboard=true;
    // debug_mouse=true;
}

void procees_sleep() {
    if (sleep_cnt > BLE_SLEEP_TITME) {
        rgblight_sethsv_noeeprom(HSV_WHITE);
        nrf_delay_ms(100);
        rgblight_sethsv_noeeprom(HSV_OFF);
        nrf_delay_ms(100);
        rgblight_sethsv_noeeprom(HSV_WHITE);
        nrf_delay_ms(100);
        rgblight_sethsv_noeeprom(HSV_OFF);
        nrf_delay_ms(100);
        sleep_mode_enter();
    }
    sleep_cnt++;
}

void matrix_scan_user_master(void) {
    process_trackball(mouse_speed);
    process_ble_status_rgblight();
    procees_sleep();

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
            layer_on(_MISC2);
            is_my_untag_ms_active = false;
        }
    }
}
