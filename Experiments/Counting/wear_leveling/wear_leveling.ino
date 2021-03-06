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
    // n cells to use --> 1/n wear per cll --> n times the life time
    const uint16_t cells_to_use = 128;
 
    // default cell to change
    uint8_t change_this_cell  = 0;
    // value of the default cell
    uint8_t change_value = EEPROM.read(change_this_cell);
 
    // will be used to aggregate the count_limit
    // must be able to hold values up to cells_to_use*255 + 1
    uint32_t count = change_value;
 
    for (uint16_t cell = 1; cell < cells_to_use; ++cell) {
        uint8_t value = EEPROM.read(cell);
 
        // determine current count by cummulating all cells
        count += value;
 
        if (value != change_value ) {
            // at the same time find at least one cell that differs
            change_this_cell = cell;
        }
    }
 
    // Either a cell differs from cell 0 --> change it
    // Otherwise no cell differs from cell 0 --> change cell 0
 
    // Since a cell might initially hold a value of -1 the % operator must be applied twice
    EEPROM.write(change_this_cell, (EEPROM.read(change_this_cell) % count_limit + 1) % count_limit);
 
    // return the new count
    return (count + 1) % count_limit;
}
 
 
void setup() {
    uint8_t count = get_next_count(20);
 
    for (uint8_t pin = 0; pin < 20; ++pin) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, pin == count);
    }
}
 
void loop() { }
