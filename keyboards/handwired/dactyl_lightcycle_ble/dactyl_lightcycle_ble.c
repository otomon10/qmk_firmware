#include "dactyl_lightcycle_ble.h"

#include "nrfx_power.h"

bool has_usb(void) {
    return (nrfx_power_usbstatus_get() == NRFX_POWER_USB_STATE_CONNECTED ||
            nrfx_power_usbstatus_get() == NRFX_POWER_USB_STATE_READY);
}
