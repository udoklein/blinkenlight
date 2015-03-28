#!/usr/bin/python
#
#   www.blinkenlight.net
#
#   Copyright 2011 Udo Klein
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program. If not, see http://www.gnu.org/licenses/


import math

# output common start of program
print """#include <MsTimer2.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

const uint8_t ports = 3;
const uint8_t brightness_levels = 32;
const uint8_t period = 39;

uint8_t const pov_pattern[ports*brightness_levels*period] PROGMEM = {"""

def cursor(phase): return phase if phase<period/2 else period-phase
def distance_to_cursor(LED, phase): return abs(LED-cursor(phase))
def brightness_by_distance(distance): return [32, 8, 1, 0, 0][distance] if distance<5 else 0
def brightness(LED, phase): return brightness_by_distance(distance_to_cursor(LED, phase))
def LEDstate(LED, phase, cycle): return 1 if cycle < brightness(LED, phase) else 0

period = 39
cycles = 32
LEDs = 20

# for each colum in the picture output 3 bytes
# that will be copied into the LED output ports
for phase in range(0, period):
    for cycle in range(0, cycles):
        line = "    0b"
        for LED in range(0, LEDs):
            if LED==6 or LED==12:
                line = line+", 0b"
            line = line + str(LEDstate(LED, phase, cycle))

    #   add a comment that makes it easier to see
    #   the intended meaning of the 3 bytes
        line = line + (", // phase {0:>2}, cycle {1:>2}".format(phase, cycle))

        print line

print """};
"""

##uint32_t duration(uint8_t pos) {
#return (sqrt(((float) 20-pos)/20)-sqrt(((float) 19-pos)/20))*500;

line = "uint16_t ms_delay[period] = {"
for phase in range(0, period):
    cursor = phase if 2*phase < period else period-phase
    delay = int(60*(math.sqrt(cursor+2)-math.sqrt(cursor+1)))
    line += "{0:>3},".format(delay)

print line+"};"
print """
volatile uint16_t base_index = 0;

void iterate() {
    static uint8_t index = 0;
    static uint16_t counter = 0;

    if (counter < ms_delay[index]) {
        ++counter;
    } else {
        counter = 0;

        base_index = index*(ports*brightness_levels);
        ++index;
        if (index == period) {
            index = 0;
        }
    }
}

void setup() {
    DDRD = 0b11111111; // set digital  0- 7 to output
    DDRB = 0b00111111; // set digital  8-13 to output
    DDRC = 0b00111111; // set digital 14-19 to output (coincidences with analog 0-5)

    MsTimer2::set(2, iterate);
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
"""
