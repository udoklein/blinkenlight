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
 
 
// http://en.wikipedia.org/wiki/Utility_frequency
// https://www.entsoe.eu/system-operations/the-frequency/
// http://www.mikrocontroller.net/topic/69366
 
#include <MsTimer2.h>
 
void iterate() {
    static int8_t phase = 19;
 
    digitalWrite(phase, LOW);
 
    --phase;
    if (phase < 0) { phase = 19; }
 
    digitalWrite(phase, HIGH);
}
 
void setup() {
    DDRD = 0b11111111; // set digital  0- 7 to output
    DDRB = 0b00111111; // set digital  8-13 to output
    DDRC = 0b00111111; // set digital 14-19 to output (coincidences with analog 0-5)
 
    // trigger each millisecond --> 20 steps in 20 milliseconds --> 50Hz
    MsTimer2::set(1, iterate);
    MsTimer2::start();
}
 
void loop() { }
