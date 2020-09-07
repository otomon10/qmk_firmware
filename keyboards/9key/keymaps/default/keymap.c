#include QMK_KEYBOARD_H

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

/* LAYER 0
 * ,-----------------------.
 * |   1   |   2   |   3   |
 * |-------+-------+-------|
 * |   4   | 5/ENT |   6   | Dbl Tap 5 for Enter
 * |-------+-------+-------|
 * |  7/0  |   8   | 9/FN  | 7/0 = Dbl Tap 7 for 0  -  9/FN = Hold 9 for FN
 * `-----------------------'
 */
[0] = LAYOUT( \
// Left
        KC_1,      KC_2,       \
        KC_3,      LT(1, KC_4),       \
        RGB_TOG,   LT(2, KC_6), \
// Right
        KC_1,      KC_2,       \
        KC_3,      LT(1, KC_4),       \
        RGB_TOG,   LT(2, KC_6) \
),

/* LAYER 1
 * ,-----------------------.
 * |  ESC  |   +   |   -   |
 * |-------+-------+-------|
 * |  BSPC |   *   |   /   |
 * |-------+-------+-------|
 * |  00   |   .   |       |
 * `-----------------------'
 */
[1] = LAYOUT( \
// Left
    RGB_M_B, RGB_M_R, \
    RGB_M_SW, KC_TRNS, \
    RGB_M_K,  RGB_M_SN,  \
// Right
    RGB_M_B, RGB_M_R, \
    RGB_M_SW, KC_TRNS, \
    RGB_M_K,  RGB_M_SN  \
),

/* LAYER 2 */
[2] = LAYOUT( \
// Left
    RGB_M_X, RGB_M_G, \
    RGB_M_T, RGB_VAI, \
    RGB_VAD,  KC_TRNS,  \
// Right
    RGB_M_X, RGB_M_G, \
    RGB_M_T, RGB_VAI, \
    RGB_VAD,  KC_TRNS  \
)

};
