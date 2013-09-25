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


const uint8_t step_delay = 500;
const uint8_t phases     = 8;
const uint8_t led_count  = 20;
const uint8_t sync_input_pin = led_count - 1;


void display_phase(uint8_t phase) {

    for (uint8_t pin=0; pin< led_count; ++pin) {
        if ((pin+phase) % phases == sync_input_pin % phases) {
            digitalWrite(pin, HIGH);
            pinMode(pin, OUTPUT);
        } else {
            if (pin > 1) { // pins 0 and 1 are pulled by the 1k resistors from the FTDI chip
                pinMode(pin, INPUT);
            }
            digitalWrite(pin, LOW);
        }
    }
}

void soft_delay(uint32_t ms) {
    uint32_t start_time = millis();
    while (millis()-start_time < ms && !digitalRead(sync_input_pin)) {};
}


void setup() {
}

void loop() {
    display_phase(0);
    delay(step_delay);
    display_phase(1);
    delay(step_delay);

    for (uint8_t phase = 2; phase < phases; ++phase) {
        display_phase(phase);
        soft_delay(step_delay);
    }
}
