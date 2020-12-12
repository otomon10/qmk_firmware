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
#include "app_ble_func.h"
#include "ble.h"
#include "keymap_def.h"
#include "pointing_device.h"
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
#define MOUSE_SPEED_SHIFT_DEFAULT 1
#define MOUSE_SPEED_SHIFT_SLOW 2
#define MS_WH_THRESHOLD_DEFAULT 4
#define MS_WH_SPEEDUP_INTERVAL 20

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
    TG_AESC,
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
uint8_t mouse_speed = MOUSE_SPEED_SHIFT_DEFAULT;
bool tg_aesc_enable;
uint8_t tg_aesc_cnt;
bool is_cspace_fn_active = false;
uint16_t cspace_fn_timer = 0;
bool is_my_tag_active = false;
uint16_t my_tag_timer = 0;
bool is_my_untag_ms_active = false;
uint16_t my_untag_ms_timer = 0;
bool enable_rgb_base = false;

// clang-format off
const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
[_BASE] = LAYOUT( \
	// Left
	KC_ESC,			KC_Q,			KC_W,		KC_E,		KC_R,		KC_T,					XXXXXXX,		XXXXXXX,		\
	KC_TAB,			KC_A,			KC_S,		KC_D,		KC_F,		KC_G,					KC_BSPACE,		MY_SPACE_FN,	\
	KC_LSHIFT,		KC_Z,			KC_X,		KC_C,		KC_V,		KC_B,					XXXXXXX,		TD(CT_ALT),		\
	RALT(KC_F7),	KC_LGUI,		KC_GRV,		C(KC_TAB),	TG_AESC,			KC_SPACE,		KC_LSHIFT,		TD(CT_CTRL),	\
	// Right
	XXXXXXX,		XXXXXXX,					KC_Y,		KC_U,		KC_I,		KC_O,		KC_P,			KC_BSLASH,		\
	MY_TAG,			KC_DELETE,					KC_H,		KC_J,		KC_K,		KC_L,		KC_SCOLON,		KC_QUOTE,		\
	MY_UNTAG_MS,	XXXXXXX,					KC_N,		KC_M,		KC_COMMA,	KC_DOT,		KC_SLASH,		KC_RSHIFT,		\
	XXXXXXX,		MO(_NUMS),		KC_ENTER,				KC_LEFT,	KC_DOWN,	KC_UP,		KC_RIGHT,		RALT(KC_F8)		\
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
	XXXXXXX,	XXXXXXX,				_______,	MS_CBTN,	_______,	_______,	_______,	_______,	\
	_______,	_______,				KC_WH_L,	KC_WH_U,	KC_WH_R,	_______,	_______,	_______,	\
	_______,	XXXXXXX,				MS_LBTN,	KC_WH_D,	MS_RBTN,	MS_SLOW,	_______,	_______,	\
	XXXXXXX,	_______,	_______,				_______,	_______,	_______,	_______,	_______		\
),

[_FN_MISC] = LAYOUT( \
	// Left
	AD_WO_L,	ADV_ID0,	ADV_ID1,	 ADV_ID2,	ADV_ID3,	ADV_ID4,				XXXXXXX,	XXXXXXX,	\
	KC_F11,		KC_F1,		KC_F2,  	KC_F3,		KC_F4,		KC_F5,					KC_DRS2,	_______,	\
	RGB_TOG,	DEL_ID0 ,	DEL_ID1,	DEL_ID2,	DEL_ID3,	DEL_ID4,				XXXXXXX,	KC_DRS1,	\
	RGB_BASE,	KC_LEFT,	KC_DOWN,	KC_UP,		KC_RIGHT,				KC_ENTER,	_______,	KC_DRS,		\
	// Right
	XXXXXXX,	XXXXXXX,				BLE_DIS,	BLE_EN,	USB_DIS,	USB_EN,	BATT_LV,	DELBNDS,	\
	_______,	KC_DMP2,				KC_F6,		KC_F7,		KC_F8,		KC_F9,		KC_F10,		KC_F12,		\
	KC_DMP1,	XXXXXXX,				_______,	_______,	_______,	_______,	_______,	_______,	\
	XXXXXXX,	_______,	_______,				KC_PSCR,	KC_INS,		_______,	AD_WO_L,	ENT_DFU 	\
),
};
// clang-format on

bool process_record_user_ble(uint16_t keycode, keyrecord_t *record) {
    char str[16];
    if (record->event.pressed) {
        switch (keycode) {
            case DELBNDS:
                delete_bonds();
                return false;
            case AD_WO_L:
                restart_advertising_wo_whitelist();
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
            case BATT_LV:
                sprintf(str, "%4dmV", get_vcc());
                send_string(str);
                return false;
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
                enable_trackball_force_move();
            } else {
                disable_trackball_force_move_with_delay();
            }
            trackball_continue_moving();
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
                enable_trackball_force_move();
            } else {
                report.buttons &= ~btn;
                disable_trackball_force_move_with_delay();
            }
            pointing_device_set_report(report);
            trackball_continue_moving();
            return false;
        }
        case MS_SLOW: {
            if (record->event.pressed) {
                mouse_speed = MOUSE_SPEED_SHIFT_SLOW;
            } else {
                mouse_speed = MOUSE_SPEED_SHIFT_DEFAULT;
            }
            return false;
        }
        case TG_AESC: {
            if (record->event.pressed) {
                if (!tg_aesc_enable) {
                    register_code(KC_RALT);
                    register_code(KC_ESC);
                    tg_aesc_enable = true;
                    tg_aesc_cnt = 0;
                } else {
                    register_code(KC_ESC);
                    tg_aesc_cnt = 0;
                }
            } else {
                unregister_code(KC_ESC);
            }
            return false;
        }
        case KC_RSHIFT:
        case KC_LSHIFT: {
            if (record->event.pressed) {
                is_shitft_pressed = true;
            } else {
                is_shitft_pressed = false;
            }
            break;
        }
        case KC_BSPACE: {
            if (record->event.pressed) {
                if (is_shitft_pressed) {
                    unregister_code(KC_LSHIFT);
                    unregister_code(KC_RSHIFT);
                    register_code(KC_DELETE);
                    // keep hold for long press
                    // register_code(KC_LSHIFT);
                    // register_code(KC_RSHIFT);
                } else {
                    register_code(KC_BSPACE);
                }
            } else {
                unregister_code(KC_DELETE);
                unregister_code(KC_BSPACE);
            }
            return false;
        }
        case KC_DELETE: {
            if (record->event.pressed) {
                if (is_shitft_pressed) {
                    unregister_code(KC_LSHIFT);
                    unregister_code(KC_RSHIFT);
                    register_code(KC_BSPACE);
                } else {
                    register_code(KC_DELETE);
                }
            } else {
                unregister_code(KC_DELETE);
                unregister_code(KC_BSPACE);
            }
            return false;
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
                is_my_untag_ms_active = false;
                disable_trackball_force_move();
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
            rgblight_sethsv_noeeprom(HSV_OBLIVION_BLUE);
            break;
        case _FN_MISC:
            rgblight_sethsv_noeeprom(HSV_OBLIVION_GREEN);
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

void matrix_scan_user_master(void) {
    process_trackball(mouse_speed);
    process_ble_status_rgblight();

    // ALT + ESC
    if (tg_aesc_enable) {
        tg_aesc_cnt++;
        if (tg_aesc_cnt > AESC_ENABLE_TIME) {
            tg_aesc_enable = false;
            unregister_code(KC_RALT);
        }
        if (is_shitft_pressed) {
            tg_aesc_cnt = 0;
        }
    }
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
            is_my_untag_ms_active = false;
            enable_trackball_force_move();
        }
    }
}
