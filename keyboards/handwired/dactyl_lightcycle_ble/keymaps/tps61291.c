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
#include "wait.h"
#include "gpio.h"
#include "adc.h"

/* voltage boost converter */
#define TPS61291_PIN 5

void init_tps61291() {
    setPinOutput(TPS61291_PIN);
    enable_voltage_boost();
}

void enable_voltage_boost() { writePinHigh(TPS61291_PIN); }

void disable_voltage_boost() { writePinLow(TPS61291_PIN); }

int get_non_boost_voltage() {
    int vcc;
    disable_voltage_boost();
    wait_ms(10);
    vcc = BMPAPI->app.get_vcc_mv();
    enable_voltage_boost();
    return vcc;
}
