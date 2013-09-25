//
//  www.blinkenlight.net
//
//  Copyright 2011 Udo Klein
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program. If not, see http://www.gnu.org/licenses/
 
 
#include <EEPROM.h>
 
uint8_t get_next_count(const uint8_t count_limit) {
    uint8_t count = (EEPROM.read(0) + 1) % count_limit;
    EEPROM.write(0, count);
    return count;
}
 
void setup() {
    uint8_t count = get_next_count(20);
 
    for (uint8_t pin = 0; pin < 20; ++pin) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, pin == count);
    }
}
 
void loop() { }
