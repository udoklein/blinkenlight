//
//  www.blinkenlight.net
//
//  Copyright 2013 Udo Klein
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
 
uint8_t setOutput(uint8_t pin) {
    pinMode(pin, OUTPUT);
    return 0;
}
 
template <uint8_t led, uint16_t d1, uint16_t d2, uint16_t d3, uint16_t d4, 
                       uint16_t d5, uint16_t d6, uint16_t d7, uint16_t d8>
void light_my_fire() {  
    static uint16_t phase = setOutput(led);
     
    phase = phase < d1+d2+d3+d4+d5+d6+d7+d8-1? phase+1: 0;
 
    digitalWrite(led, phase < d1                  ? HIGH:
                      phase < d1+d2               ? LOW:
                      phase < d1+d2+d3            ? HIGH:
                      phase < d1+d2+d3+d4         ? LOW:
                      phase < d1+d2+d3+d4+d5      ? HIGH:
                      phase < d1+d2+d3+d4+d5+d6   ? LOW:
                      phase < d1+d2+d3+d4+d5+d6+d7? HIGH:
                                                    LOW);                                                   
} 
 
void blink() {
    light_my_fire< 0,  200, 2800,  200, 2800,  200, 5800,    0,    0>();  // Norderney
    light_my_fire< 4, 3000, 3000, 3000, 3000,    0, 8500,    0,    0>();  // Pilsum
    light_my_fire< 8,  700, 2300,  700, 2300,  700, 2300,  700, 5300>();  // Campen
    light_my_fire<12, 6000, 6000,    0,    0,    0,    0,    0,    0>();  // Schillig
    light_my_fire<16, 6000, 3000,    0,    0,    0,    0,    0,    0>();  // Voslap
    light_my_fire<19, 6000, 1000,    0,    0,    0,    0,    0,    0>();  // Tossens
}
 
void setup() {
    MsTimer2::set(1, blink);
    MsTimer2::start();
}
 
void loop() {}
