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
 
 
//  input capture = pin8 --> now how to exploit the display as good as possible 
 
// The following layout will be used ('X' indicates on, ' ' indicates off),
// * indicates IO pins
 
// Pins
// 01 00 08 --> used of serial connection and input capture IO
//  *  *  *
 
// Pins
// 07 06 05 04 03 | Unit of Deviation
//              X |     1 mHz         --> this is quite normal            
//           X  X |    10 mHz         --> this is also common 
//        X  X  X |   100 mHz         --> deviations >200mHz are not good and >800mHz are really really bad
//     X  X  X  X |  1000 mHz = 1 Hz  --> this is already in a black down
//  X  X  X  X  X | 10000 mHz = 10 Hz --> this is unheard of
 
const uint8_t pin_magnitude_low  = 3;
const uint8_t pin_magnitude_high = 7;
 
const uint8_t pin_l = 19;
const uint8_t pin_r = 9;
 
const uint8_t input_capture_pin = 8;
 
// Pins 
// 19 18 17 16 15 14 13 12 11 10 09 | Absolute Deviation   | Sign of Deviation
//  X                               | >  0 units, < 1 units | to slow
//  X  X                            | >= 1 units, < 2 units | to slow  
//  X  X  X                         | >= 2 units, < 3 units | to slow  
//  X  X  X  X                      | >= 3 units, < 4 units | to slow  
//  X     X  X  X                   | >= 4 units, < 5 units | to slow  
//  X        X  X  X                | >= 5 units, < 6 units | to slow  
//  X           X  X  X             | >= 6 units, < 7 units | to slow  
//  X              X  X  X          | >= 7 units, < 8 units | to slow  
//  X                 X  X  X       | >= 8 units, < 9 units | to slow  
//  X                    X  X  X    | >= 9 units, <10 units | to slow   
//  X                       X  X  X | >=10 units, <11 units | to slow
//  X                          X  X | >=11 units            | to slow
 
//                                X | >  0 units,< 1 units | to fast
//                             X  X | >= 1 unit, < 2 units | to fast
//                          X  X  X | >= 2 unit, < 3 units | to fast
//                       X  X  X  X | >= 3 unit, < 4 units | to fast
//                    X  X  X     X | >= 4 unit, < 5 units | to fast
//                 X  X  X        X | >= 5 unit, < 6 units | to fast
//              X  X  X           X | >= 6 unit, < 7 units | to fast
//           X  X  X              X | >= 7 unit, < 8 units | to fast
//        X  X  X                 X | >= 8 unit, < 9 units | to fast
//     X  X  X                    X | >= 9 unit, <10 units | to fast
//  X  X  X                       X | >=10 unit, <11 units | to fast
//  X  X                          X | >=11 unit            | to fast
 
         
 
// helper type for efficient access of the counter structure 
typedef union {
    uint32_t clock_ticks;
    struct {
        uint8_t  byte_0;
        uint8_t  byte_1;
        uint16_t word_1;
    };  
} converter;
 
 
// The timer values will be initialized nowhere.
// This works because we are intested in the differences only.
// The first differences will be meaningless due to lack
// of initialization. However they would be meaningless anyway
// because the very first capture has (by definition) no predecessor.
// So the lack of initialization semantically reflects this.
volatile converter input_capture_time;
volatile uint16_t timer1_overflow_count;
 
// 0 indicates "invalid"
volatile uint32_t period_length = 0;
volatile bool next_sample_ready = false;
 
ISR(TIMER1_OVF_vect) {
    ++timer1_overflow_count;
}
 
ISR(TIMER1_CAPT_vect) { 
    static uint32_t previous_capture_time = 0;
   
    // according to the datasheet the low byte must be read first
    input_capture_time.byte_0 = ICR1L; 
    input_capture_time.byte_1 = ICR1H;   
                             
    if ((TIFR1 & (1<<TOV1) && input_capture_time.byte_1 < 128)) { 
        // we have a timer1 overflow AND 
        // input capture time is so low that we must assume that it 
        // was wrapped around
       
        ++timer1_overflow_count;     
         
        // we clear the overflow bit in order to not trigger the 
        // overflow ISR, otherwise this overflow would be
        // counted twice
        TIFR1 = (1<<TOV1);
    }   
    input_capture_time.word_1 = timer1_overflow_count;
     
    period_length = input_capture_time.clock_ticks - previous_capture_time;
    previous_capture_time = input_capture_time.clock_ticks;
     
    next_sample_ready = true;
}                 
                   
void initialize_timer1() {
  
    // Timer1: "normal mode", no automatic toggle of output pins
    //                        wave form generation mode  with Top = 0xffff
    TCCR1A = 0;                
 
    // Timer 1: input capture noise canceler active
    //          input capture trigger on rising edge
    //          clock select: no prescaler, use system clock 
    TCCR1B = (1<<ICNC1) | (1<< ICES1) | (1<<CS10);
     
    // Timer1: enable input capture and overflow interrupts
    TIMSK1 = (1<<ICIE1) | (1<<TOIE1);
 
    // Timer1: clear input capture and overflow flags by writing a 1 into them    
    TIFR1 = (1<<ICF1) | (1<<TOV1) ;            
}    
                   
void visualize_frequency_deviation(const uint8_t target_frequency, const uint32_t period_length) {
   
    const int64_t deviation_1000 = 1000*(int64_t)F_CPU / period_length - 1000*(int64_t)target_frequency;
     
    //Serial.print(F("period length  "));
    //Serial.println(period_length);
     
    static int8_t sign = 1;
    if (deviation_1000 != 0) {
        // only compute new sign for frequency deviation != 0
        sign = deviation_1000 >= 0? 1: -1;
    }
     
    //Serial.print(F("deviation: "));
    Serial.println((int32_t)deviation_1000);
             
    const uint64_t value = abs(deviation_1000);     
         
    // magnitude is static in order to introduce some hysteresis into the computation
    static uint8_t magnitude = 1;
    magnitude = value >= 12000? 5:
                value >= 10000? (magnitude >= 5? 5: 4):
                value >=  1200? 4:
                value >=  1000? (magnitude >= 4? 4: 3):
                value >=   120? 3:
                value >=   100? (magnitude >= 3? 3: 2):
                value >=    12? 2:
                value >=    10? (magnitude >= 2? 2: 1):
                                1; 
 
    //Serial.print(F("magnitude: ")); Serial.println(magnitude);
     
    for (uint8_t pin = pin_magnitude_low; pin <= pin_magnitude_high; ++pin) {
        digitalWrite(pin, pin-pin_magnitude_low < magnitude);
    }
     
     
    uint32_t divider = 1;
    for (uint8_t power = 1; power < magnitude; ++power) {    
        divider *= 10;
    }
    
    const uint8_t output_value = value / divider;
 
    //Serial.print(F("sign: ")); Serial.println(sign);
    //Serial.print(F("value: ")); Serial.println(output_value, DEC);
 
    if (sign <= 0) {                
        digitalWrite(pin_r, HIGH);
         
        uint8_t pin = pin_r + 1;        
        while (pin <= pin_l && pin <= pin_r - 3 + output_value) {
            digitalWrite(pin++, LOW);
        }
        while (pin <= pin_l && pin <= pin_r + output_value ) {
            digitalWrite(pin++, HIGH);          
        }
        while (pin <= pin_l) {
            digitalWrite(pin++, LOW);
        }
    } else {
        digitalWrite(pin_l, HIGH);
         
        uint8_t pin = pin_l - 1;        
        while (pin >= pin_r && pin >= pin_l + 3 - output_value) {
            digitalWrite(pin--, LOW);
        }
        while (pin >= pin_r && pin >= pin_l - output_value) {
            digitalWrite(pin--, HIGH);          
        }
        while (pin >= pin_r) {
            digitalWrite(pin--, LOW);
        }      
    }    
}  
                   
void setup() {
   
    Serial.begin(115200);
   
    for (uint8_t pin = pin_magnitude_low; pin <= pin_magnitude_high; ++pin) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
    }
   
    for (uint8_t pin = pin_r; pin <= pin_l; ++pin) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
    }
 
    // just to indicate which pins will be the output pins
    // this "output test" makes it easier to wire the circuit
    delay(1000);
 
    pinMode(input_capture_pin, INPUT);
    digitalWrite(input_capture_pin, HIGH);
 
    initialize_timer1();
}
 
const uint8_t target_frequency = 50;
const uint8_t sample_buffer_size = 50;
 
uint32_t sample_buffer[sample_buffer_size];
 
void loop() {  
    static uint8_t sample_index = 0;
     
    if (next_sample_ready) {
        next_sample_ready = false;
         
        cli();
        sample_buffer[sample_index] = period_length;    
        sei();
        sample_index = sample_index > 0? sample_index - 1: sample_buffer_size - 1;
     
        uint32_t average_period_length = 0;
        for (uint8_t index = 0; index < sample_buffer_size; ++index) {
            average_period_length += sample_buffer[index];
        }
        average_period_length /= sample_buffer_size;        
         
        visualize_frequency_deviation(target_frequency, average_period_length);
    }    
}
