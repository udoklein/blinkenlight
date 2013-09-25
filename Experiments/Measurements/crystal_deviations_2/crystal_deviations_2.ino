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
 
 
#include <avr/io.h>
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
 
#define LEDstate_ramp(LED, phase) (LED<=phase? 1: 0)
#define LEDstate_phased_pulse(LED, phase) (LED==phase? 1: 0)
 
#define PortAssignment(LEDstate, phase) \
    PORTD = \
        (LEDstate( 0, phase)   ) + \
        (LEDstate( 1, phase)<<1) + \
        (LEDstate( 2, phase)<<2) + \
        (LEDstate( 3, phase)<<3) + \
        (LEDstate( 4, phase)<<4) + \
        (LEDstate( 5, phase)<<5) + \
        (LEDstate( 6, phase)<<6) + \
        (LEDstate( 7, phase)<<7);  \
    PORTB = \
        (LEDstate( 8, phase)   ) + \
        (LEDstate( 9, phase)<<1) + \
        (LEDstate(10, phase)<<2) + \
        (LEDstate(11, phase)<<3) + \
        (LEDstate(12, phase)<<4) + \
        (LEDstate(13, phase)<<5);  \
    PORTC = \
        (LEDstate(14, phase)   ) + \
        (LEDstate(15, phase)<<1) + \
        (LEDstate(16, phase)<<2) + \
        (LEDstate(17, phase)<<3) + \
        (LEDstate(18, phase)<<4) + \
        (LEDstate(19, phase)<<5);
 
#define divider1(LEDstate, phase) PortAssignment(LEDstate, phase);
 
#define divider10(LEDstate, phase)   \
    PortAssignment(LEDstate, phase); \
    PortAssignment(LEDstate, phase); \
    PortAssignment(LEDstate, phase); \
    PortAssignment(LEDstate, phase); \
    PortAssignment(LEDstate, phase); \
    PortAssignment(LEDstate, phase); \
    PortAssignment(LEDstate, phase); \
    PortAssignment(LEDstate, phase); \
    PortAssignment(LEDstate, phase); \
    PortAssignment(LEDstate, phase);
 
#define set_phase(divider, LEDstate, phase) divider(LEDstate, phase);
 
#define iterate(divider, LEDstate) \
        cli();  \
    l0: set_phase(divider, LEDstate, 0); \
        set_phase(divider, LEDstate, 1); \
        set_phase(divider, LEDstate, 2); \
        set_phase(divider, LEDstate, 3); \
        set_phase(divider, LEDstate, 4); \
        set_phase(divider, LEDstate, 5); \
        set_phase(divider, LEDstate, 6); \
        set_phase(divider, LEDstate, 7); \
        set_phase(divider, LEDstate, 8); \
        set_phase(divider, LEDstate, 9); \
        set_phase(divider, LEDstate,10); \
        set_phase(divider, LEDstate,11); \
        set_phase(divider, LEDstate,12); \
        set_phase(divider, LEDstate,13); \
        set_phase(divider, LEDstate,14); \
        set_phase(divider, LEDstate,15); \
        set_phase(divider, LEDstate,16); \
        set_phase(divider, LEDstate,17); \
        set_phase(divider, LEDstate,18); \
        set_phase(divider, LEDstate,19);    goto l0;
 
void slow_sweep() {
 
    for (uint8_t phase=0; phase<20; ++phase) {
        PortAssignment(LEDstate_phased_pulse, phase);
        delay(50);
    }
 
    iterate(divider10, LEDstate_phased_pulse);
}
 
void fast_sweep() {
    for (uint8_t pass=0; pass<3; ++pass) {
        for (uint8_t phase=0; phase<20; ++phase) {
            PortAssignment(LEDstate_phased_pulse, phase);
            delay(16);
        }
    }
 
    iterate(divider1, LEDstate_phased_pulse);
}
 
void slow_ramp() {
    for (uint8_t phase=0; phase<20; ++phase) {
        PortAssignment(LEDstate_ramp, phase);
        delay(50);
    }
 
    iterate(divider10, LEDstate_ramp);
}
 
void fast_ramp() {
    for (uint8_t pass=0; pass<3; ++pass) {
        for (uint8_t phase=0; phase<20; ++phase) {
            PortAssignment(LEDstate_ramp, phase);
            delay(16);
        }
    }
 
    iterate(divider1, LEDstate_ramp);
}
 
 
void setup() {
    DDRD = 0b11111111; // set digital  0- 7 to output
    DDRB = 0b00111111; // set digital  8-13 to output
    DDRC = 0b00111111; // set digital 14-19 to output (coincidences with analog 0-5)
 
    typedef void (*sweep_pattern)(void);
    sweep_pattern pattern[] = { fast_sweep, slow_sweep, fast_ramp, slow_ramp };
    sweep_pattern sweep = pattern[get_next_count(sizeof(pattern)/sizeof(pattern[0]))];
    sweep();
}
 
void loop() { }
