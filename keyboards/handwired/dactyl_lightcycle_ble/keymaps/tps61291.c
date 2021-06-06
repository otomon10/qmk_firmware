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

#include "tps61291.h"
#include "gpio.h"
#include "timer.h"
#include "bmp.h"

#define TPS61291_PIN 5
#define BATTERY_CHECK_INTERVAL 60000       // 1 min
#define DISABLE_VOLTAGE_BOOST_DURATION 10  // 10 ms

uint32_t battery_check_timer;
int      non_boost_voltage = 0;

void init_tps61291() {
    setPinOutput(TPS61291_PIN);
    enable_voltage_boost();

    battery_check_timer = timer_read32() - BATTERY_CHECK_INTERVAL;
}

void enable_voltage_boost() { writePinHigh(TPS61291_PIN); }

void disable_voltage_boost() { writePinLow(TPS61291_PIN); }

int get_non_boost_voltage() { return non_boost_voltage; }

void update_non_boost_voltage() {
    static bool     voltage_boost       = true;
    static uint32_t voltage_boost_timer = 0;

    if (timer_elapsed(battery_check_timer) >= BATTERY_CHECK_INTERVAL) {
        if (voltage_boost) {
            disable_voltage_boost();

            voltage_boost_timer = timer_read32();
            voltage_boost       = false;
        } else {
            if (timer_elapsed(voltage_boost_timer) >=
                DISABLE_VOLTAGE_BOOST_DURATION) {
                non_boost_voltage = BMPAPI->app.get_vcc_mv();

                enable_voltage_boost();
                voltage_boost       = true;
                battery_check_timer = timer_read32();
            }
        }
    }
}

void battery_task() {
    if (is_usb_connected()) {
        return;
    }

    update_non_boost_voltage();

    /* prevent over discharge */
    if (non_boost_voltage != 0 && non_boost_voltage < 2000) {
        BMPAPI->app.enter_sleep_mode();
    }
}
