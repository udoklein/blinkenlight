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
 
 
void setup() {
    for (uint8_t pin=2; pin<20; ++pin) {
        pinMode(pin, OUTPUT);   
    }
    Serial.begin(9600);
    Serial.println("ready, send characters a-s to control output");
    set_volume(0);
}
 
void set_volume(uint8_t volume) {
    volume+= 2;
    for (uint8_t pin=2; pin<20; ++pin) {
        digitalWrite(pin, pin<volume);
    }
}
 
void loop() {
    uint8_t volume = Serial.read() - 'a';
    if (volume < 't'-'a') {
        set_volume(volume);
    }
}
