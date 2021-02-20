/*
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

#include "ble_helper.h"

#include "app_ble_func.h"
#include "ble_central.h"
#include "flash.h"
#include "keymap_def.h"

pm_peer_id_t g_destination_peer_id;
bool changed_peer;

void ble_connect_last_peer() {
    uint8_t *addr = FLASH_ADDR_BLE_LAST_CONNECTION_PEER_ID;
    uint8_t peer_id = flash_read_byte(addr);
    nrf_delay_ms(100);  // need this

    ble_connect_id(peer_id);
}

void ble_connect_id(uint8_t id) {
    uint32_t peer_cnt = get_peer_cnt();
    pm_peer_id_t peer_id = get_peer_id();
    // dprintf("id = %d, peer_id = %d, peer_cnt = %d", id, peer_id, peer_cnt);

    if (!is_connected_slave()) {
        /* give priority to slave */
        return;
    }

    if (peer_cnt == 0) {
        /* no device */
        return;
    }

    if (id < peer_cnt && id != peer_id) {
        restart_advertising_id(id);
        g_destination_peer_id = id;

        /* force ble enabled */
        set_usb_enabled(false);
        set_ble_enabled(true);

        changed_peer = true;
    }
}

void ble_connect_advertising_wo_whitelist() {
    if (!is_connected_slave()) {
        /* give priority to slave */
        return;
    }

    g_destination_peer_id = get_peer_cnt();  // new device
    restart_advertising_wo_whitelist();
    changed_peer = true;
}

bool is_ble_connected() {
    pm_peer_id_t peer_id = get_peer_id();
    return peer_id == g_destination_peer_id;
}

void process_ble_status_rgblight() {
    if (is_ble_connected()) {
        if (changed_peer) {
            /* blink LED peer id times */
            int i;
            rgblight_sethsv_noeeprom(HSV_OFF);
            nrf_delay_ms(100);
            for (i = 0; i < g_destination_peer_id + 1; ++i) {
                rgblight_sethsv_noeeprom(HSV_OBLIVION_GREEN);
                nrf_delay_ms(100);
                rgblight_sethsv_noeeprom(HSV_OFF);
                nrf_delay_ms(100);
            }
            changed_peer = false;

            uint8_t *addr = FLASH_ADDR_BLE_LAST_CONNECTION_PEER_ID;
            uint8_t value = g_destination_peer_id;
            flash_write_byte(addr, value);
        }
    }
}
