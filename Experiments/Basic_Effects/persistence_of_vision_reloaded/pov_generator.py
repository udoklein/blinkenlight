#!/usr/bin/python
# -*- coding: utf-8 -*-
 
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
 
 
import sys
from PIL import Image
 
# double check that there is at least one argument (the file name)
if len(sys.argv) != 2:
    print "Please specify a source filename"
    sys.exit(-1)
 
# determine source file name
sourcefilename = sys.argv[1]
# strip extension
name = sourcefilename.split(".")[0]
 
# let PIL determine the proper graphics file type
image = Image.open(sourcefilename)
 
# convert image to black and white
image = image.convert("1")
 
# get hold of image size
(xsize, ysize) = image.size
# ensure height is 20 pixels (=number of LEDs)
if ysize != 20:
    print "Image height must be 20 pixels but is {0} pixels".format(ysize)
 
# output common start of program
print """#include <MsTimer2.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
 
#define PARSE(pattern) ((pattern>>14) & 0b111111), ((pattern>>8) & 0b111111), (pattern & 0b11111111)
 
uint8_t pov_pattern[] PROGMEM = {"""
 
# generate the bit pattern for the LEDs
for x in range(0, xsize):
    line = "    PARSE( 0b"
    for y in range(0, ysize):
        line = line + ('0' if image.getpixel((x, ysize-1-y)) != 0 else '1')
    line = line + " ),"
 
    print line
 
# output common end of program
print """};
 
void blink() {
    static uint16_t index = 0;
 
    PORTC = pgm_read_byte(pov_pattern+(index++));
    PORTB = pgm_read_byte(pov_pattern+(index++));
    PORTD = pgm_read_byte(pov_pattern+(index++));
 
    if (index >= sizeof(pov_pattern)) { index = 0; }
}
 
void setup() {
    DDRD = 0b11111111; // set digital  0- 7 to output
    DDRB = 0b00111111; // set digital  8-13 to output
    DDRC = 0b00111111; // set digital 14-19 to output (coincidences with analog 0-5)
 
    MsTimer2::set(2, blink);
    MsTimer2::start();
}
 
void loop() { }
"""
