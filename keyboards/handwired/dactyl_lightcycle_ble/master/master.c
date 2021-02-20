#include "app_ble_func.h"
#include "ble_central.h"
#include "flash.h"
#include "keymap_def.h"
#include "keymaps/default/ble_helper.h"
#include "keymaps/default/paw3204.h"
#include "keymaps/default/tps61291.h"
#include "keymaps/default/trackball.h"
#include "matrix.h"
#include "rgblight.h"
#include "wait.h"

bool has_usb(void);
extern void matrix_scan_user_master(void);

void matrix_init_user(void) {
    /* init trackball */
    init_paw3204();
    /* init touch sensor */
    init_ttp223();
    /* init voltage boost converter */
    init_tps61291();
    /* init fds */
    flash_init();
}

void matrix_scan_user(void) {
    static int cnt = 0;
    static bool init_rgblight = false;
    static bool init_ble_connect = false;
    static bool init_battery_check = false;

    /* init rgblight did not work well in keyboard_post_init_user(), so do it
     * here */
    if (!init_rgblight) {
        if (cnt == 0) {
            rgblight_enable_noeeprom();
            nrf_delay_ms(10);  // need this
            rgblight_sethsv_noeeprom(HSV_WHITE);
        } else if (cnt > 30) {
            rgblight_sethsv_noeeprom(HSV_OFF);
            nrf_delay_ms(10);
            init_rgblight = true;
        }
    }

    /* prevent over discharge */
    if (!init_battery_check) {
        int i = 0;
        int voltage = get_non_boost_voltage();

        /* led alert  */
        if (voltage < 2250) {
            for (i = 0; i < 10; ++i) {
                rgblight_sethsv_noeeprom(HSV_RED);
                nrf_delay_ms(100);
                rgblight_sethsv_noeeprom(HSV_OFF);
                nrf_delay_ms(100);
            }
        } else if (voltage < 2380) {
            for (i = 0; i < 5; ++i) {
                rgblight_sethsv_noeeprom(HSV_OBLIVION_YELLOW);
                nrf_delay_ms(100);
                rgblight_sethsv_noeeprom(HSV_OFF);
                nrf_delay_ms(100);
            }
        }

        /* force sleep */
        if (voltage < 2000) {
            sleep_mode_enter();
        }
        init_battery_check = true;
    }

    if (is_connected_slave()) {
        if (!init_ble_connect) {
            bool enable_usb = has_usb();
            if (!enable_usb) {
                /* automatic connect last peer */
                ble_connect_last_peer();
            }
            init_ble_connect = true;
        }
    }

    /* automatic setting during USB insertion and removal */
    if (cnt % 50 == 0) {
        static bool enable_usb_prev;
        bool enable_usb = has_usb();
        if (enable_usb_prev != enable_usb) {
            if (enable_usb) {
                set_usb_enabled(true);
                set_ble_enabled(false);

            } else {
                set_usb_enabled(false);
                set_ble_enabled(true);

                /* automatic connect last peer */
                if (is_connected_slave()) {
                    ble_connect_last_peer();
                }
            }
            enable_usb_prev = enable_usb;
        }
    }

    matrix_scan_user_master();
    flash_update();

    cnt++;
}
