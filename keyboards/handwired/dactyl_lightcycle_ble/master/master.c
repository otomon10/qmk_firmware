#include "app_ble_func.h"
#include "keymaps/default/ble_helper.h"
#include "keymaps/default/paw3204.h"
#include "matrix.h"
#include "rgblight.h"
#include "wait.h"

bool has_usb(void);
extern void matrix_scan_user_master(void);

void matrix_init_user(void) {
    /* init trackball */
    init_paw3204();
}

void matrix_scan_user(void) {
    static int cnt;
    static bool init_rgblight = false;
    static bool init_ble_connect = false;

    /* init rgblight did not work well in keyboard_post_init_user(), so do it
     * here */
    if (!init_rgblight) {
        rgblight_enable_noeeprom();
        nrf_delay_ms(10);  // need this
        rgblight_sethsv_noeeprom(0, 0, RGBLIGHT_LIMIT_VAL);
        init_rgblight = true;
    }

    if (!init_ble_connect) {
        bool enable_usb = has_usb();
        if (!enable_usb) {
            /* automatic connect ble peer id 0 */
            ble_connect_id(0);
        }
        init_ble_connect = true;
    }

    /* automatic setting during USB insertion and removal */
    if (cnt++ % 50 == 0) {
        static bool enable_usb_prev;
        bool enable_usb = has_usb();
        if (enable_usb_prev != enable_usb) {
            if (enable_usb) {
                set_usb_enabled(true);
                set_ble_enabled(false);

            } else {
                set_usb_enabled(false);
                set_ble_enabled(true);

                /* automatic connect ble peer id 0 */
                ble_connect_id(0);
            }
            enable_usb_prev = enable_usb;
        }
    }

    matrix_scan_user_master();
}