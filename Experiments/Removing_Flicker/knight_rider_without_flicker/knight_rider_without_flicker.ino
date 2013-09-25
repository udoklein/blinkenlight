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
 
 
#include <MsTimer2.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
 
const uint8_t ports = 3;
const uint8_t brightness_levels = 32;
const uint8_t period = 38;
const uint8_t half_period = period>>1;  // 19
 
 
#define cursor(phase) (phase<half_period? phase: period-phase)
 
#define distance_to_cursor(LED, phase) abs(LED-cursor(phase))
 
#define brightness_by_distance(distance) \
    (distance==0? 32:                   \
        (distance== 1? 16:              \
            (distance==2? 8:            \
                (distance==3? 2: 1))))
 
#define brightness(LED, phase) (brightness_by_distance(distance_to_cursor(LED, phase)))
 
// cycle will loop from 0 to brightness_level
#define LEDstate(LED, phase, cycle) (cycle > brightness(LED, phase)? 0: 1)
 
#define PortstateC(phase, cycle) (  \
    (LEDstate(14, phase, cycle)   ) + \
    (LEDstate(15, phase, cycle)<<1) + \
    (LEDstate(16, phase, cycle)<<2) + \
    (LEDstate(17, phase, cycle)<<3) + \
    (LEDstate(18, phase, cycle)<<4) + \
    (LEDstate(19, phase, cycle)<<5)  )
 
#define PortstateB(phase, cycle) (  \
    (LEDstate( 8, phase, cycle)   ) + \
    (LEDstate( 9, phase, cycle)<<1) + \
    (LEDstate(10, phase, cycle)<<2) + \
    (LEDstate(11, phase, cycle)<<3) + \
    (LEDstate(12, phase, cycle)<<4) + \
    (LEDstate(13, phase, cycle)<<5)  )
 
#define PortstateD(phase, cycle) (  \
    (LEDstate( 0, phase, cycle)   ) + \
    (LEDstate( 1, phase, cycle)<<1) + \
    (LEDstate( 2, phase, cycle)<<2) + \
    (LEDstate( 3, phase, cycle)<<3) + \
    (LEDstate( 4, phase, cycle)<<4) + \
    (LEDstate( 5, phase, cycle)<<5) + \
    (LEDstate( 6, phase, cycle)<<6) + \
    (LEDstate( 7, phase, cycle)<<7)  )
 
#define Ports(phase, cycle) PortstateC(phase, cycle), PortstateB(phase, cycle), PortstateD(phase, cycle)
#define pwm(phase) \
    Ports(phase,  0), Ports(phase,  1), Ports(phase,  2), Ports(phase,  3), \
    Ports(phase,  4), Ports(phase,  5), Ports(phase,  6), Ports(phase,  7), \
    Ports(phase,  8), Ports(phase,  9), Ports(phase, 10), Ports(phase, 11), \
    Ports(phase, 12), Ports(phase, 13), Ports(phase, 14), Ports(phase, 15), \
    Ports(phase, 16), Ports(phase, 17), Ports(phase, 18), Ports(phase, 19), \
    Ports(phase, 20), Ports(phase, 21), Ports(phase, 22), Ports(phase, 23), \
    Ports(phase, 24), Ports(phase, 25), Ports(phase, 26), Ports(phase, 27), \
    Ports(phase, 28), Ports(phase, 29), Ports(phase, 30), Ports(phase, 31)
 
 
uint8_t pov_pattern[ports*brightness_levels*period] PROGMEM = {
    pwm( 0), pwm( 1), pwm( 2), pwm( 3), pwm( 4), 
    pwm( 5), pwm( 6), pwm( 7), pwm( 8), pwm( 9),
    pwm(10), pwm(11), pwm(12), pwm(13), pwm(14),
    pwm(15), pwm(16), pwm(17), pwm(18), pwm(19),
    pwm(20), pwm(21), pwm(22), pwm(23), pwm(24),
    pwm(25), pwm(26), pwm(27), pwm(28), pwm(29),
    pwm(30), pwm(31), pwm(32), pwm(33), pwm(34),
    pwm(35), pwm(36), pwm(37)
};
 
 
volatile uint16_t base_index = 0;
 
void iterate() {
    base_index += ports*brightness_levels;
    if (base_index >= sizeof(pov_pattern)) { base_index = 0; }
}
 
void setup() {
    DDRD = 0b11111111; // set digital  0- 7 to output 
    DDRB = 0b00111111; // set digital  8-13 to output
    DDRC = 0b00111111; // set digital 14-19 to output (coincidences with analog 0-5)
 
    MsTimer2::set(60, iterate);
    MsTimer2::start();
}
 
void loop() {
    static uint16_t index;
    cli();
    index = base_index;
    sei();
 
    for (uint8_t cycle=0; cycle<brightness_levels; ++cycle) {
        PORTC = pgm_read_byte(pov_pattern+(index++));
        PORTB = pgm_read_byte(pov_pattern+(index++));
        PORTD = pgm_read_byte(pov_pattern+(index++));
    }
}
