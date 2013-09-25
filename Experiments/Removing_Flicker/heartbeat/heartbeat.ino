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
#include <MsTimer2.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
 
const uint8_t patterns = 3;
const uint8_t ports = 3;
const uint8_t brightness_levels = 32;
const uint8_t period = 38;
const uint8_t half_period = period>>1;  // 19
 
 
// phase will loop from 0 to period
#define cursor(phase) (phase<half_period? phase: period-phase)
 
#define distance_to_cursor(LED, phase) abs(LED-cursor(phase))
 
#define brightness_by_distance(distance) \
    (distance==0? 32:                    \
        (distance==1? 16:                \
            (distance==2? 8:             \
                (distance==3? 2: 1))))
 
#define b1(LED, phase) (brightness_by_distance(distance_to_cursor(LED, phase)))
#define p1(LED, phase) b1(LED, phase)
 
#define b2(LED, phase) (brightness_by_distance(distance_to_cursor(LED, ((phase+half_period)%period))))
#define p2(LED, phase) max(b1(LED, phase), b2(LED, phase))
 
#define tmp(LED, phase) ((phase<half_period? phase: period-phase)-abs(2*LED-19)+3)
#define p3(LED, phase) (tmp(LED, phase)? (3*tmp(LED, phase))/2: 0)
 
#define brightness_LED(pattern, LED)                        \
    enum brightness_##pattern##_##LED {                     \
        brightness_##pattern##_##LED##_0  = pattern(LED, 0), \
        brightness_##pattern##_##LED##_1  = pattern(LED, 1), \
        brightness_##pattern##_##LED##_2  = pattern(LED, 2), \
        brightness_##pattern##_##LED##_3  = pattern(LED, 3), \
        brightness_##pattern##_##LED##_4  = pattern(LED, 4), \
        brightness_##pattern##_##LED##_5  = pattern(LED, 5), \
        brightness_##pattern##_##LED##_6  = pattern(LED, 6), \
        brightness_##pattern##_##LED##_7  = pattern(LED, 7), \
        brightness_##pattern##_##LED##_8  = pattern(LED, 8), \
        brightness_##pattern##_##LED##_9  = pattern(LED, 9), \
        brightness_##pattern##_##LED##_10 = pattern(LED,10), \
        brightness_##pattern##_##LED##_11 = pattern(LED,11), \
        brightness_##pattern##_##LED##_12 = pattern(LED,12), \
        brightness_##pattern##_##LED##_13 = pattern(LED,13), \
        brightness_##pattern##_##LED##_14 = pattern(LED,14), \
        brightness_##pattern##_##LED##_15 = pattern(LED,15), \
        brightness_##pattern##_##LED##_16 = pattern(LED,16), \
        brightness_##pattern##_##LED##_17 = pattern(LED,17), \
        brightness_##pattern##_##LED##_18 = pattern(LED,18), \
        brightness_##pattern##_##LED##_19 = pattern(LED,19), \
        brightness_##pattern##_##LED##_20 = pattern(LED,20), \
        brightness_##pattern##_##LED##_21 = pattern(LED,21), \
        brightness_##pattern##_##LED##_22 = pattern(LED,22), \
        brightness_##pattern##_##LED##_23 = pattern(LED,23), \
        brightness_##pattern##_##LED##_24 = pattern(LED,24), \
        brightness_##pattern##_##LED##_25 = pattern(LED,25), \
        brightness_##pattern##_##LED##_26 = pattern(LED,26), \
        brightness_##pattern##_##LED##_27 = pattern(LED,27), \
        brightness_##pattern##_##LED##_28 = pattern(LED,28), \
        brightness_##pattern##_##LED##_29 = pattern(LED,29), \
        brightness_##pattern##_##LED##_30 = pattern(LED,30), \
        brightness_##pattern##_##LED##_31 = pattern(LED,31), \
        brightness_##pattern##_##LED##_32 = pattern(LED,32), \
        brightness_##pattern##_##LED##_33 = pattern(LED,33), \
        brightness_##pattern##_##LED##_34 = pattern(LED,34), \
        brightness_##pattern##_##LED##_35 = pattern(LED,35), \
        brightness_##pattern##_##LED##_36 = pattern(LED,36), \
        brightness_##pattern##_##LED##_37 = pattern(LED,37), \
        brightness_##pattern##_##LED##_38 = pattern(LED,38) }
 
#define brightness_enumeration(pattern) \
    brightness_LED(pattern,  0); \
    brightness_LED(pattern,  1); \
    brightness_LED(pattern,  2); \
    brightness_LED(pattern,  3); \
    brightness_LED(pattern,  4); \
    brightness_LED(pattern,  5); \
    brightness_LED(pattern,  6); \
    brightness_LED(pattern,  7); \
    brightness_LED(pattern,  8); \
    brightness_LED(pattern,  9); \
    brightness_LED(pattern, 10); \
    brightness_LED(pattern, 11); \
    brightness_LED(pattern, 12); \
    brightness_LED(pattern, 13); \
    brightness_LED(pattern, 14); \
    brightness_LED(pattern, 15); \
    brightness_LED(pattern, 16); \
    brightness_LED(pattern, 17); \
    brightness_LED(pattern, 18); \
    brightness_LED(pattern, 19)
 
// cycle will loop from 0 to brightness_level
#define LEDstate(pattern, LED, phase, cycle) (( cycle > brightness_##pattern##_##LED##_##phase )? 0: 1)
 
#define PortstateC(pattern, phase, cycle) (  \
    (LEDstate(pattern, 14, phase, cycle)   ) + \
    (LEDstate(pattern, 15, phase, cycle)<<1) + \
    (LEDstate(pattern, 16, phase, cycle)<<2) + \
    (LEDstate(pattern, 17, phase, cycle)<<3) + \
    (LEDstate(pattern, 18, phase, cycle)<<4) + \
    (LEDstate(pattern, 19, phase, cycle)<<5)  )
 
#define PortstateB(pattern, phase, cycle) (  \
    (LEDstate(pattern,  8, phase, cycle)   ) + \
    (LEDstate(pattern,  9, phase, cycle)<<1) + \
    (LEDstate(pattern, 10, phase, cycle)<<2) + \
    (LEDstate(pattern, 11, phase, cycle)<<3) + \
    (LEDstate(pattern, 12, phase, cycle)<<4) + \
    (LEDstate(pattern, 13, phase, cycle)<<5)  )
 
#define PortstateD(pattern, phase, cycle) (  \
    (LEDstate(pattern,  0, phase, cycle)   ) + \
    (LEDstate(pattern,  1, phase, cycle)<<1) + \
    (LEDstate(pattern,  2, phase, cycle)<<2) + \
    (LEDstate(pattern,  3, phase, cycle)<<3) + \
    (LEDstate(pattern,  4, phase, cycle)<<4) + \
    (LEDstate(pattern,  5, phase, cycle)<<5) + \
    (LEDstate(pattern,  6, phase, cycle)<<6) + \
    (LEDstate(pattern,  7, phase, cycle)<<7)  )
 
 
#define Ports(pattern, phase, cycle) PortstateC(pattern, phase, cycle), PortstateB(pattern, phase, cycle), PortstateD(pattern, phase, cycle)
#define pwm(pattern, phase) \
    Ports(pattern, phase,  0), Ports(pattern, phase,  1), Ports(pattern, phase,  2), Ports(pattern, phase,  3), \
    Ports(pattern, phase,  4), Ports(pattern, phase,  5), Ports(pattern, phase,  6), Ports(pattern, phase,  7), \
    Ports(pattern, phase,  8), Ports(pattern, phase,  9), Ports(pattern, phase, 10), Ports(pattern, phase, 11), \
    Ports(pattern, phase, 12), Ports(pattern, phase, 13), Ports(pattern, phase, 14), Ports(pattern, phase, 15), \
    Ports(pattern, phase, 16), Ports(pattern, phase, 17), Ports(pattern, phase, 18), Ports(pattern, phase, 19), \
    Ports(pattern, phase, 20), Ports(pattern, phase, 21), Ports(pattern, phase, 22), Ports(pattern, phase, 23), \
    Ports(pattern, phase, 24), Ports(pattern, phase, 25), Ports(pattern, phase, 26), Ports(pattern, phase, 27), \
    Ports(pattern, phase, 28), Ports(pattern, phase, 29), Ports(pattern, phase, 30), Ports(pattern, phase, 31)
 
#define the_pattern(pattern) \
    pwm(pattern, 0), pwm(pattern, 1), pwm(pattern, 2), pwm(pattern, 3), pwm(pattern, 4), \
    pwm(pattern, 5), pwm(pattern, 6), pwm(pattern, 7), pwm(pattern, 8), pwm(pattern, 9), \
    pwm(pattern,10), pwm(pattern,11), pwm(pattern,12), pwm(pattern,13), pwm(pattern,14), \
    pwm(pattern,15), pwm(pattern,16), pwm(pattern,17), pwm(pattern,18), pwm(pattern,19), \
    pwm(pattern,20), pwm(pattern,21), pwm(pattern,22), pwm(pattern,23), pwm(pattern,24), \
    pwm(pattern,25), pwm(pattern,26), pwm(pattern,27), pwm(pattern,28), pwm(pattern,29), \
    pwm(pattern,30), pwm(pattern,31), pwm(pattern,32), pwm(pattern,33), pwm(pattern,34), \
    pwm(pattern,35), pwm(pattern,36), pwm(pattern,37)
 
 
brightness_enumeration(p1);
brightness_enumeration(p2);
brightness_enumeration(p3);
 
uint8_t pov_pattern[patterns*ports*brightness_levels*period] PROGMEM = {
    the_pattern(p1),
    the_pattern(p2),
    the_pattern(p3)
};
 
 
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
 
 
static uint16_t mode_offset = 0;
volatile uint16_t base_index = 0;
 
void iterate() {
    base_index += ports*brightness_levels;
    if (base_index >= sizeof(pov_pattern)/patterns) { base_index = 0; }
}
 
void setup() {
    mode_offset = get_next_count(patterns)*ports*brightness_levels*period;
 
    DDRD = 0b11111111; // set digital  0- 7 to output
    DDRB = 0b00111111; // set digital  8-13 to output
    DDRC = 0b00111111; // set digital 14-19 to output (coincidences with analog 0-5)
 
    MsTimer2::set(60, iterate);
    MsTimer2::start();
}
 
void loop() {
    static uint16_t index;
    cli();
    index = base_index + mode_offset;
    sei();
 
    for (uint8_t cycle=0; cycle<brightness_levels; ++cycle) {
        PORTC = pgm_read_byte(pov_pattern+(index++));
        PORTB = pgm_read_byte(pov_pattern+(index++));
        PORTD = pgm_read_byte(pov_pattern+(index++));
    }
}
