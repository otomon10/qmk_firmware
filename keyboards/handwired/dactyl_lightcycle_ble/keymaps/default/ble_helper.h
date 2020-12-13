#pragma once

#include "quantum.h"

#define FLASH_ADDR_BLE_LAST_CONNECTION_PEER_ID 0

void ble_connect_last_peer();
void ble_connect_id(uint8_t id);
bool is_ble_connected();
void process_ble_status_rgblight();
