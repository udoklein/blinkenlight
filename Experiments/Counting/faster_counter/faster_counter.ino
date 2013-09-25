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
 
 
#define COUNT_1                asm volatile ("out 0x09, r20");
#define COUNT_3    COUNT_1;    asm volatile ("out 0x09, r21"); COUNT_1;
#define COUNT_7    COUNT_3;    asm volatile ("out 0x09, r22"); COUNT_3;
#define COUNT_15   COUNT_7;    asm volatile ("out 0x09, r23"); COUNT_7;
#define COUNT_31   COUNT_15;   asm volatile ("out 0x09, r24"); COUNT_15;
#define COUNT_63   COUNT_31;   asm volatile ("out 0x09, r25"); COUNT_31;
#define COUNT_127  COUNT_63;   asm volatile ("out 0x09, r26"); COUNT_63;
#define COUNT_255  COUNT_127;  asm volatile ("out 0x09, r27"); COUNT_127;
#define COUNT_511  COUNT_255;  asm volatile ("out 0x03, r20"); COUNT_255;
#define COUNT_1023 COUNT_511;  asm volatile ("out 0x03, r21"); COUNT_511;
#define COUNT_2047 COUNT_1023; asm volatile ("out 0x03, r22"); COUNT_1023;
#define COUNT_4095 COUNT_2047; asm volatile ("out 0x03, r23"); COUNT_2047;
 
void count_8191() { COUNT_4095; asm volatile ("out 0x03, r24"); COUNT_4095; }
 
#define COUNT_16383  count_8191(); asm volatile ("out 0x03, r25"); count_8191();
#define COUNT_32767  COUNT_16383;  asm volatile ("out 0x06, r20"); COUNT_16383;
#define COUNT_65535  COUNT_32767;  asm volatile ("out 0x06, r21"); COUNT_32767;
#define COUNT_131071 COUNT_65535;  asm volatile ("out 0x06, r22"); COUNT_65535;
#define COUNT_262143 COUNT_131071; asm volatile ("out 0x06, r23"); COUNT_131071;
#define COUNT_524287 COUNT_262143; asm volatile ("out 0x06, r24"); COUNT_262143;
#define COUNT_524288 COUNT_524287; asm volatile ("out 0x06, r25");
 
#define TWICE(a) a a
 
void setup() {
    DDRD = 0b11111111; // set digital  0- 7 to output
    DDRB = 0b00111111; // set digital  8-13 to output
    DDRC = 0b00111111; // set digital 14-19 to output (coincidences with analog 0-5)
 
    // ensure we do not get interrupted during prescaler manipulation
    cli();
 
/*
    // activate this block by removing the previous comment
    // set clock prescaler as desired to slow down the timer
    const uint8_t clock_prescaler_1   = 0;
    const uint8_t clock_prescaler_2   = 1;
    const uint8_t clock_prescaler_4   = 2;
    const uint8_t clock_prescaler_8   = 3;
    const uint8_t clock_prescaler_16  = 4;
    const uint8_t clock_prescaler_32  = 5;
    const uint8_t clock_prescaler_64  = 6;
    const uint8_t clock_prescaler_128 = 7;
    const uint8_t clock_prescaler_256 = 8;
 
    // prepare to set clock prescaler: write CLKPCE bit to one and all the other to zero
    CLKPR = 1<<CLKPCE;
    // set clock prescaler immediately after preparing to do so
    CLKPR = clock_prescaler_256;
/**/
    // do not enable interrupts back again because we do not want
    // interrupts to slow us any down
}
 
void loop() {
     asm volatile ("ldi r20, 0x01");
     asm volatile ("ldi r21, 0x02");
     asm volatile ("ldi r22, 0x04");
     asm volatile ("ldi r23, 0x08");
     asm volatile ("ldi r24, 0x10");
     asm volatile ("ldi r25, 0x20");
     asm volatile ("ldi r26, 0x40");
     asm volatile ("ldi r27, 0x80");
 
    l0:
        TWICE(TWICE(TWICE(TWICE(TWICE(COUNT_524288)))));
    goto l0;
}
