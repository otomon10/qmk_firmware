
#include "app_ble_func.h"
#include "ble_slave.h"
#include "bootloader.h"
#include "keymap_def.h"
#include "keymaps/default/tps61291.h"
#include "matrix.h"
#include "nrf.h"
#include "nrf_power.h"
#include "rgblight.h"
#include "wait.h"

#define BLE_SLEEP_TITME (3600 * 0.5) /* sleep in 0.5 min  */

bool has_usb(void);

void matrix_init_user(void) {
    /* init voltage boost converter */
    init_tps61291();
}

void matrix_scan_user() {
    static int cnt = 0;
    static int master_disconnect_cnt = 0;
    static bool init_rgblight = true;
    static bool init_connect_rgblight = true;

    if (init_rgblight) {
        rgblight_enable_noeeprom();
        nrf_delay_ms(10);  // need this
        init_rgblight = false;
    }

    /* If the slave has a USB connection, it will be forced into dfu mode */
    if (cnt < 100 && has_usb()) {
        bootloader_jump();
    }

    if (is_connected_master()) {
        if (init_connect_rgblight) {
            rgblight_sethsv_noeeprom(0, 0, 255);
            nrf_delay_ms(500);
            rgblight_sethsv_noeeprom(0, 0, 0);
            nrf_delay_ms(10);  // need this

            master_disconnect_cnt = 0;

            init_connect_rgblight = false;
        }
    } else {
        /* process sleep */
        if (master_disconnect_cnt > BLE_SLEEP_TITME) {
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
        master_disconnect_cnt++;

        init_connect_rgblight = true;
    }

    cnt++;
}
