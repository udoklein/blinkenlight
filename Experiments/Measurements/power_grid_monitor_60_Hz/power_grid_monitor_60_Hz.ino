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
 
ISR(TIMER1_COMPA_vect) {
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
 
    // Trigger with 60Hz == each 1/60s == each 16 000 000/60 cycles.
    // We have 20 phases per period, each phase will last 16 000 000/(60*20) == 13 333 + 1/3 cycle.
    // Our frequency generator will be to fast by 1/3 * 20 * 60 cycles/s == 400 cycles/s. Thus the generated frequency will be slightly to high.
    // The error is about 400 / 16 000 000 == 25ppm.
    // Since 25ppm is in the order of magnitude of the crystal's deviation we will not compensate for this.
 
    // disable timer1 interrupts
    TIMSK1 = 0;
 
    // Mode 4, CTC using OCR1A | set prescaler to 1
    TCCR1A = 0;
    TCCR1B = (1<<WGM12) | (1<<CS10);
 
    // Set OCR1A for running at 1200 Hz (because we have 20 discrete phases per period and want to have 60 Hz)
    OCR1A = (F_CPU / (60*20)) - 1;
 
    // enable match interrupts
    TIMSK1 = 1<<OCIE1A;
}
 
void loop() { }
