/*
Copyright 2018 Sekigon
Copyright 2021 otomon10

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

#include "ws2812.h"
#include "spi.h"
#include "apidef.h"
#include "gpio.h"

typedef void (*spi_ws2812_t)(uint8_t *p_tx_buffer, size_t tx_length,
                             uint8_t cs_pin);

#ifndef WS2812_DATA
#    define WS2812_DATA RGB_DI_PIN
#endif

#ifndef WS2812_SCLK
#    define WS2812_SCLK 6  // dummy pin
#endif

void ws2812_setleds(LED_TYPE *ledarray, uint16_t number_of_leds) {
    ws2812_setleds_pin(ledarray, number_of_leds,
                       BMPAPI->app.get_config()->led.pin);
}

void ws2812_spi_soft_simplex(uint8_t *p_tx_buffer, size_t tx_length,
                             uint8_t cs_pin) {
    // clang-format off
    bmp_api_spim_config_t config = {
        .freq = SPI_FREQ_4M,    // 0.25us
        .miso = 0xFF,
        .mosi = WS2812_DATA,
        .sck = WS2812_SCLK,
        .mode = CONFIG_SPI_MODE
    };
    // clang-format on

    // configure data pin as output mode
    BMPAPI->spim.init(&config);

    // FIXME: PIN19 stuck low
    setPinInputHigh(19);

    BMPAPI->spim.start(p_tx_buffer, tx_length, NULL, 0, cs_pin);
}

spi_ws2812_t spi_ws2812 = ws2812_spi_soft_simplex;

void ws2812_setleds_pin(LED_TYPE *ledarray, uint16_t number_of_leds,
                        uint8_t pinmask) {
    uint8_t       led[RGBLED_NUM * 3 * 8] = {0};
    uint8_t *     p_dat                   = &led[0];
    const uint8_t t0                      = 0x80;  // 1000 0000, H:0.25us
    const uint8_t t1                      = 0xE0;  // 1110 0000, H:0.75us

    for (int i = 0; i < RGBLED_NUM; i++) {
        uint8_t mask = 0x80;
        for (int j = 0; j < 8; j++) {
            *p_dat++ = (ledarray[i].g & (mask >> j)) ? t1 : t0;
        }
        mask = 0x80;
        for (int j = 0; j < 8; j++) {
            *p_dat++ = (ledarray[i].r & (mask >> j)) ? t1 : t0;
        }
        mask = 0x80;
        for (int j = 0; j < 8; j++) {
            *p_dat++ = (ledarray[i].b & (mask >> j)) ? t1 : t0;
        }
    }
    spi_ws2812(led, sizeof(led), 0xFF);
}
