#pragma once

#include <stdint.h>

void flash_update(void);
void flash_init(void);
uint8_t flash_read_byte(const uint8_t *addr);
void flash_write_byte(uint8_t *addr, uint8_t value);
