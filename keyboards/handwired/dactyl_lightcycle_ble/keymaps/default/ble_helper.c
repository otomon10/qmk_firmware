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
#include "keymap_def.h"

pm_peer_id_t g_destination_peer_id;
bool enable_animation;

void ble_connect_id(uint8_t id) {
    uint32_t peer_cnt = get_peer_cnt();
    pm_peer_id_t peer_id = get_peer_id();
    // dprintf("id = %d, peer_id = %d, peer_cnt = %d", id, peer_id, peer_cnt);
    if (id < peer_cnt && id != peer_id) {
        restart_advertising_id(id);
        g_destination_peer_id = id;

        /* force ble enabled */
        set_usb_enabled(false);
        set_ble_enabled(true);

        enable_animation = true;
    }
}

bool is_ble_connected() {
    pm_peer_id_t peer_id = get_peer_id();
    return peer_id == g_destination_peer_id;
}

void process_ble_status_rgblight() {
    if (is_ble_connected()) {
        if (enable_animation) {
            rgblight_sethsv_noeeprom(HSV_OBLIVION_GREEN);
            nrf_delay_ms(100);
            rgblight_sethsv_noeeprom(HSV_OFF);
            nrf_delay_ms(100);
            rgblight_sethsv_noeeprom(HSV_OBLIVION_GREEN);
            nrf_delay_ms(100);
            rgblight_sethsv_noeeprom(HSV_OFF);
            nrf_delay_ms(100);
            rgblight_sethsv_noeeprom(HSV_OBLIVION_GREEN);
            nrf_delay_ms(100);
            rgblight_sethsv_noeeprom(HSV_OFF);
            nrf_delay_ms(100);
            enable_animation = false;
        }
    }
}
