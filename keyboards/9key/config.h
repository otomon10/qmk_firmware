/*
Copyright 2012 Jun Wako <wakojun@gmail.com>

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

#ifndef CONFIG_H
#define CONFIG_H

#include "config_common.h"

/* USB Device descriptor parameter */
#define VENDOR_ID       0xFEED
#define PRODUCT_ID      0x0007
#define DEVICE_VER 		0x0001
#define MANUFACTURER    otomon
#define PRODUCT         dactyl
#define DESCRIPTION     dactyl lightcycle // Charleston, SC Meetup Handout

/* key matrix size */
#define MATRIX_ROWS 8   // Split setting Left:4 Right:4
#define MATRIX_COLS 8

/* 9Key PCB default pin-out */
#define MATRIX_ROW_PINS { D4, C6, D7, E6 }
#define MATRIX_COL_PINS { F4, F5, F6, F7, B1, B3, B2, B6 }
#define MATRIX_COL_PINS_RIGHT { B6, B2, B3, B1, F7, F6, F5, F4}    // wiring error
#define UNUSED_PINS

/* ws2812 RGB LED */
#define RGB_DI_PIN D3
#define RGBLED_SPLIT {29, 28}
#define RGBLED_NUM 57    // Number of LEDs
//#define RGBLIGHT_ANIMATIONS
#define RGBLIGHT_LIMIT_VAL 100 // For current limitation

/* COL2ROW or ROW2COL */
#define DIODE_DIRECTION COL2ROW

#define TAPPING_TERM 200

// split keybord
#define USE_I2C

#if 1 // write right hands
#define MASTER_RIGHT
#endif

#endif
