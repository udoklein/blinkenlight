//
//  www.blinkenlight.net
//
//  Copyright 2012 Udo Klein
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
 
// Configuration for supported target frequencies
// Notice that increasing the frequencies may increase the sampling error.
// Decreasing the frequencies below 13Hz will not work out of the box
// because timer1 has only a 16bit counter.
const uint8_t frequency[] = {15, 24, 25, 30, 50, 60};
 
#include <EEPROM.h>
 
uint8_t get_next_count(const uint8_t count_limit) {
    // n cells to use --> 1/n wear per cell --> n times the life time
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
    EEPROM.write(change_this_cell, ((uint16_t)EEPROM.read(change_this_cell) + 1) % count_limit);
 
    // return the new count
    return (count + 1) % count_limit;
}
 
ISR(TIMER1_COMPA_vect) {
    static int8_t phase = 0;
 
    digitalWrite(phase, LOW);
 
    ++phase;
    if (phase > 19) { phase = 0; }
 
    digitalWrite(phase, HIGH);
}
 
void setup_timer1(const uint8_t target_frequency) {
    // Run with n = 20*target_frequency Hz 
     
    // We want 20 phases per period, each phase will last 16 000 000/(n*20)
    // Since gcd(n, 16000000) == 3 for the frequencies (15, 24, 30, 60) the generator may be somewhat to slow.
    // For example for 60 Hz (the worst case) we will have 16 000 000 - 60*20*floor(16 000 000/60/20) == 400.
    // Thus the worst case error is at most 400 ticks per second or 25ppm.
    // Since 25ppm is in the order of magnitude of the crystal's deviation we will not compensate for this.
    // For the other frequencies gcd == 0 and thus there is no error but the crystal deviation.
 
    // disable timer1 interrupts
    TIMSK1 = 0;
 
    // Mode 4, CTC using OCR1A | set prescaler to 1
    TCCR1A = 0;
    TCCR1B = (1<<WGM12) | (1<<CS10);
 
    // Set OCR1A for running at 20 times the frequency to cater for the 20 phases    
    OCR1A = (F_CPU / (target_frequency*20));
 
    // enable match interrupts
    TIMSK1 = 1<<OCIE1A;
}
 
void visualize_frequency(const uint8_t target_frequency) {
    // lower decimal digit of target frequency
    for (uint8_t led=0; led<10; ++led) {
        digitalWrite(led, led< target_frequency % 10);
    }
   
    // higher decimal digit of target frequency
    for (uint8_t led=0; led<10; ++led) {
        digitalWrite(led+10, led< target_frequency / 10);
    }
     
    delay(2000);
}
 
void set_prescaler(const uint8_t prescaler) {
    // ensure we do not get interrupted during prescaler manipulation
    cli();
 
    // prepare to set clock prescaler: write CLKPCE bit to one and all the other to zero
    CLKPR = 1<<CLKPCE;
    // set clock prescaler immediately after preparing to do so
    CLKPR = prescaler;
 
    sei();
}
 
void visualize_scan_direction() {
    const uint8_t clock_prescaler_1   = 0;
    const uint8_t clock_prescaler_2   = 1;
    const uint8_t clock_prescaler_4   = 2;
    const uint8_t clock_prescaler_8   = 3;
    const uint8_t clock_prescaler_16  = 4;
    const uint8_t clock_prescaler_32  = 5;
    const uint8_t clock_prescaler_64  = 6;
    const uint8_t clock_prescaler_128 = 7;
    const uint8_t clock_prescaler_256 = 8;
 
    set_prescaler(clock_prescaler_32);
     
    // this will take 80ms * 32 = 2.56s
    delay(80);
     
    set_prescaler(clock_prescaler_1);
}
 
 
void set_all_pins_to_output() {
    DDRD = 0b11111111; // set digital  0- 7 to output
    DDRB = 0b00111111; // set digital  8-13 to output
    DDRC = 0b00111111; // set digital 14-19 to output (coincidences with analog 0-5)
}
 
void setup() {
    set_all_pins_to_output();
     
    uint8_t target_frequency = frequency[get_next_count(sizeof(frequency)/sizeof(frequency[0]))];    
    visualize_frequency(target_frequency);
     
    setup_timer1(target_frequency);        
    visualize_scan_direction();
}
 
void loop() { }
