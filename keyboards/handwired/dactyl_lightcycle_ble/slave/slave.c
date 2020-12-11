
#include "matrix.h"

#include "app_ble_func.h"
#include "bootloader.h"
#include "wait.h"

#include "nrf.h"
#include "nrf_power.h"

#include "rgblight.h"

bool has_usb(void);

void matrix_scan_user() {
    static int cnt;
    static bool init_rgblight = false;

    /* init rgblight did not work well in keyboard_post_init_user(), so do it
     * here */
    if (!init_rgblight) {
        rgblight_enable_noeeprom();
        nrf_delay_ms(10);  // need this
        rgblight_sethsv_noeeprom(0, 0, 255);
        init_rgblight = true;
    }

    /* If the slave has a USB connection, it will be forced into dfu mode */
    if (cnt++ % 50 == 0 && has_usb()) {
        rgblight_sethsv_noeeprom(302, 255, 255);
        nrf_delay_ms(10);
        bootloader_jump();
    }
}
