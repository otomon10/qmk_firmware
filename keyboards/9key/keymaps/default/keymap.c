#include QMK_KEYBOARD_H

#include "paw3204.h"
#include "pointing_device.h"

enum layer_names {
    _BASE,
    _MOUSE,
    _NUMS,
    _MISC
};

enum my_keycodes {
    MS_LBTN = SAFE_RANGE,
    MS_RBTN,
    MS_CBTN,
    MS_SLOW,
    TG_AESC
};

#define KC_MOUSE    MO(_MOUSE)
#define KC_NUMS     MO(_NUMS)
#define KC_MISK     MO(_MISC)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

[_BASE] = LAYOUT( \
    // Left
    KC_GRV,	    KC_Q,	    KC_W,	    KC_E,	    KC_R,	    KC_T,	                XXXXXXX,  	XXXXXXX,    \
    KC_TAB,     KC_A,	    KC_S,	    KC_D,	    KC_F,   	KC_G,	                KC_BSPACE,	TG_AESC,    \
    KC_LSHIFT,	KC_Z,	    KC_X,	    KC_C,	    KC_V,   	KC_B,	                XXXXXXX,  	C(KC_TAB),  \
    KC_LCTRL,	KC_LGUI,	KC_LALT,	KC_HOME,	KC_END, 	            KC_SPACE,   KC_NUMS, 	KC_ESC,     \
    // Right
    XXXXXXX,    XXXXXXX,            	KC_Y,   	KC_U,	    KC_I,	    KC_O,	    KC_P,	    KC_BSLASH,	\
    KC_MISK,	KC_DELETE,          	KC_H,   	KC_J,	    KC_K,   	KC_L,	    KC_SCOLON,	KC_QUOTE,	\
    KC_RALT,  	XXXXXXX,  	            KC_N,   	KC_M,	    KC_COMMA,	KC_DOT, 	KC_SLASH,	KC_RSHIFT,	\
    XXXXXXX,    KC_NUMS, 	KC_ENTER,	            KC_LEFT,	KC_DOWN,	KC_UP,	    KC_RIGHT,	KC_RCTRL   \

),

[_NUMS] = LAYOUT( \
    // Left
    KC_F1,	    KC_F2,	    KC_F3,	    KC_F4,	    KC_F5,	    KC_F6,	                XXXXXXX,    XXXXXXX, \
    _______,	KC_1,	    KC_2,	    KC_3,	    KC_4,	    KC_5,	                _______,	_______, \
    _______,	_______,	_______,	_______,	_______,	_______,				XXXXXXX,	_______, \
    _______,	_______,	_______,	_______,	_______,				_______,	_______,	_______, \
    // Right
    XXXXXXX,	XXXXXXX,				KC_F7,      KC_F8,   	KC_F9,    	KC_F10,   	KC_F11,   	KC_F12, \
    _______,	_______,   	            KC_6,		KC_7,   	KC_8,   	KC_9,   	KC_0,	    _______, \
    _______,	XXXXXXX,				_______,	KC_MINS,	KC_EQL,	    KC_LBRC,	KC_RBRC,	_______, \
    XXXXXXX,	_______,	_______,				KC_HOME,    KC_PGDN,    KC_PGUP,    KC_END, 	_______ \
),

[_MOUSE] = LAYOUT( \
    // Left
    _______,	_______,	_______,	_______,	_______,	_______,				XXXXXXX,	XXXXXXX, \
    _______,	_______,	_______,	_______,	_______,	_______,				_______,	_______, \
    _______,	_______,	_______,	_______,	_______,	_______,				XXXXXXX,	_______, \
    _______,	_______,	_______,	_______,	_______,				_______,	_______,	_______, \
    // Right
    XXXXXXX,	XXXXXXX,	            _______,	KC_WH_U,	_______,	_______,	_______,    _______, \
    _______,	_______,	            KC_WH_L,	KC_WH_D,	KC_WH_R,	_______,	_______,    _______, \
    _______,	XXXXXXX,	            MS_LBTN,	MS_CBTN,	MS_RBTN,	_______,	_______,    _______, \
    XXXXXXX,	_______,	_______,	            _______,	_______,	MS_SLOW,	_______,    _______ \
),

[_MISC] = LAYOUT( \
    // Left
    RGB_TOG,	RGB_MOD,	RGB_HUI,	RGB_SAI,	RGB_VAI,	_______,				XXXXXXX,	XXXXXXX, \
    _______,	_______,	_______,	_______,	_______,	_______,				_______,	_______, \
    _______,	_______,	_______,	_______,	_______,	_______,				XXXXXXX,	_______, \
    _______,	_______,	_______,	_______,	_______,				_______,	_______,	_______, \
    // Right
    XXXXXXX,	XXXXXXX,				_______,	_______,	_______,	_______,	_______,	_______, \
    _______,	_______,				_______,	_______,	_______,	_______,	_______,	_______, \
    _______,	XXXXXXX,				_______,	_______,	_______,	_______,	_______,	_______, \
    XXXXXXX,	_______,	_______,				KC_PSCR,	KC_INS,	    _______,	_______,	_______ \
),

};

void matrix_init_user(void) {
    // trackball
    init_paw3204();
}

void keyboard_post_init_user(void) {
    // Customise these values to desired behaviour
    //debug_enable=true;
    //debug_matrix=true;
    //debug_keyboard=true;
    //debug_mouse=true;
}

#define MOUSE_CNT_MAX 100
#define MOUSE_MOVE_CNT 5
#define MOUSE_SPEED_SHIFT_DEFAULT 1
#define MOUSE_SPEED_SHIFT_SLOW    2
uint8_t mouse_move_cnt;
uint8_t mouse_stop_cnt;
uint8_t mouse_slow = MOUSE_SPEED_SHIFT_DEFAULT;

#define AESC_ENABLE_TIME 50
bool tg_aesc_enable;
uint8_t tg_aesc_cnt;

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case KC_WH_U:
        case KC_WH_D:
        case KC_WH_L:
        case KC_WH_R:
        {
            // keep moving
            mouse_stop_cnt = 0;
            break;
        }
        case MS_LBTN:
        case MS_RBTN:
        case MS_CBTN:
        {
            // click, drag & drop
            report_mouse_t currentReport = pointing_device_get_report();
            if (record->event.pressed) {
                switch(keycode) {
                case MS_LBTN: currentReport.buttons |= MOUSE_BTN1; break;
                case MS_RBTN: currentReport.buttons |= MOUSE_BTN2; break;
                case MS_CBTN: currentReport.buttons |= MOUSE_BTN3; break;
                }
            } else {
                switch(keycode) {
                case MS_LBTN: currentReport.buttons &= ~MOUSE_BTN1; break;
                case MS_RBTN: currentReport.buttons &= ~MOUSE_BTN2; break;
                case MS_CBTN: currentReport.buttons &= ~MOUSE_BTN3; break;
                }
            }
            pointing_device_set_report(currentReport);
            pointing_device_send();
            return false;
        }
        case MS_SLOW:
        {
            if (record->event.pressed) {
                mouse_slow = MOUSE_SPEED_SHIFT_SLOW;
            } else {
                mouse_slow = MOUSE_SPEED_SHIFT_DEFAULT;
            }
            return false;
        }
        case TG_AESC:
        {
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
    }
    return true;
}

uint32_t layer_state_set_user(uint32_t state) {
    switch (biton32(state)) {
    case _BASE:
        rgblight_sethsv_noeeprom(HSV_WHITE);
        break;
    case _NUMS:
        rgblight_sethsv_noeeprom(HSV_ORANGE);
        break;
    case _MOUSE:
        rgblight_sethsv_noeeprom(HSV_MAGENTA);
        break;
    case _MISC:
        rgblight_sethsv_noeeprom(HSV_CHARTREUSE);
        break;
    }
  return state;
}

void matrix_scan_user(void) {
    static int  cnt;
    static bool paw_ready;
    report_mouse_t mouse_rep;

    if (cnt++ % 50 == 0) {
        uint8_t pid = read_pid_paw3204();
        if (pid == 0x30) {
            dprint("paw3204 OK\n");
            paw_ready = true;
        } else {
            dprintf("paw3204 NG:%d\n", pid);
            paw_ready = false;
        }
    }

    if (paw_ready) {
        uint8_t stat;
        int8_t x, y;

        read_paw3204(&stat, &x, &y);

        mouse_rep = pointing_device_get_report();
        mouse_rep.h = 0;
        mouse_rep.v = 0;
        mouse_rep.x = -y >> mouse_slow;
        mouse_rep.y = x >> mouse_slow;

        if (cnt % 10 == 0) {
            dprintf("stat:%3d btn:%4d x:%4d y:%4d move:%3d stop:%3d\n", stat, mouse_rep.buttons, mouse_rep.x, mouse_rep.y, mouse_move_cnt, mouse_stop_cnt);
        }

        if (stat & 0x80) {
            pointing_device_set_report(mouse_rep);
            if(mouse_move_cnt < MOUSE_CNT_MAX) mouse_move_cnt++;
            if(mouse_move_cnt > MOUSE_MOVE_CNT) {

                mouse_stop_cnt = 0;
            }
        } else {
            mouse_move_cnt = 0;
            if(mouse_stop_cnt < MOUSE_CNT_MAX) mouse_stop_cnt++;
        }

        if (mouse_stop_cnt != MOUSE_CNT_MAX) {
            layer_on(_MOUSE);
        } else {
            layer_off(_MOUSE);
        }
    }

    // ALT + ESC
    if (tg_aesc_enable) {
        tg_aesc_cnt++;
        if (tg_aesc_cnt > AESC_ENABLE_TIME) {
            tg_aesc_enable = false;
            unregister_code(KC_RALT);
        }
    }
}


