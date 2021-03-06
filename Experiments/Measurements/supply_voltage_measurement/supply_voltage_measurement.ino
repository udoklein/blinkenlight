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
 
// TRICK17 is a macro that just repeats its argument.
// From the point of view of the C / C++ language
// TRICK17 does nothing at all. However it tricks
// the Arduino IDE. That is whatever is wrapped by
// the trick macro will not be recognized as a function.
// This in turn will disable the auto prototyping of
// the IDE. Thus it becomes possible to use self 
// defined types in functions signatures without
// refraining to additional files.
// If something comes up with a better name for this
// trick I would be more than happy to use this name.
#define TRICK17(x) x
 
uint32_t power_of_ten(uint8_t exponent) {
    uint32_t result = 1;
     
    while (exponent > 0) {
        --exponent;
        result *= 10;
    }
    return result;
}
 
typedef uint32_t fixed_point;
 
// notice 2^32 = 4 294 967 296 ~ 4*10^9
// 10^3*10^4*phases = 1.6 * 10^8 ==> fits into uint32_t
// ==> At most one more digit might be possible but we will not push this to the limits.
//     It is somewhat pointless anyway because we are right now at the edge of reasonable
//     precision.
 
const uint8_t max_integer_digits    = 3;
const uint8_t max_fractional_digits = 4;
// constant value for the number 1 in a fixed point represenation with
// max_fractional_digits behind the dot
const fixed_point fixed_point_1 = (fixed_point)power_of_ten(max_fractional_digits);
 
 
char get_next_available_character() {
    while (!Serial.available()) {}
     
    return Serial.read();
}
 
TRICK17(fixed_point parse()) {
    const char decimal_separator     = '.';
    const char terminator[] = " \t\n\r";
   
    enum parser_state { parse_integer_part                = 0,
                        parse_fractional_part             = 1,
                        error_duplicate_decimal_separator = 2, 
                        error_invalid_character           = 3,
                        error_missing_input               = 4,
                        error_to_many_decimal_digits      = 5,
                        error_to_many_fractional_digits   = 6 };
                           
    while (true) {
 
        fixed_point value = 0;
        uint8_t parsed_digits = 0;        
        parser_state state = parse_integer_part;
     
        while (state == parse_integer_part || state == parse_fractional_part) {
         
            const char c = get_next_available_character();        
            if (c == decimal_separator) {
                if (state == parse_integer_part) { 
                   state = parse_fractional_part;
                   parsed_digits = 0;
                } else {
                    state = error_duplicate_decimal_separator;
                }
            } else            
            if (strchr(terminator, c) != NULL) {
                if (state == parse_integer_part && parsed_digits == 0) {
                    state = error_missing_input;
                } else {
                    return value * (fixed_point)power_of_ten(max_fractional_digits - (state == parse_integer_part? 0: parsed_digits));                                                     
                }
            } else           
            if (c >= '0' and c <= '9') {
                ++parsed_digits;
                value = value * 10 + c - '0';                                     
                if (state == parse_integer_part && parsed_digits > max_integer_digits) {
                    state = error_to_many_decimal_digits;
                } else
                if (state == parse_fractional_part && parsed_digits > max_fractional_digits) {
                    state = error_to_many_fractional_digits;
                }                                
            } else {
                state = error_invalid_character;
            }        
        }
        Serial.print(F("Error: "));
        Serial.println(state);
    }    
}
 
 
const uint8_t phases = 16;
const uint64_t system_clock_frequency = ((uint64_t)F_CPU) * fixed_point_1;
const fixed_point default_frequency = 1 * fixed_point_1;  // 1 Hz
 
const uint16_t delay_block_tick_size = 30000; // must be below 2^15-1 in order for tail ticks to fit into uint16_t
// total delay time will be delay_block_tick_size * delay_blocks + tail_ticks + nominator/denominator
volatile uint32_t delay_blocks = 0;
volatile uint16_t tail_ticks   = 1000;  // will always be <= 2*delay_block_tick_size
volatile fixed_point denominator = 1;
volatile fixed_point nominator = 0;
volatile boolean reset_timer = true;
 
void advance_phase() {
    const uint8_t max_led = 19;
   
    static uint8_t phase = max_led;
    static uint8_t next_phase = max_led-1;
     
    digitalWrite(phase, LOW);    
    digitalWrite(next_phase, HIGH);
     
    phase = next_phase;    
    next_phase = next_phase > max_led - phases + 1? next_phase - 1: max_led;  
}
 
void set_timer_cycles(uint16_t cycles) {    
    OCR1A = cycles - 1;
}
 
ISR(TIMER1_COMPA_vect) {
    // To decrease phase jitter "next_phase" is always precomputed.
    // Thus the start of the ISR till the manipulation of the IO pins 
    // will always take the same amount of time.
        
    static uint32_t blocks_to_delay;
    static fixed_point accumulated_fractional_ticks;
 
    if (reset_timer) {
        reset_timer = false;
         
        blocks_to_delay = 0;
        accumulated_fractional_ticks = 0;
    }
 
    if (blocks_to_delay == 0) {    
        advance_phase();              
         
        blocks_to_delay = delay_blocks;
        if (blocks_to_delay > 0) {
            set_timer_cycles(delay_block_tick_size);          
        }        
    } else {
        --blocks_to_delay;
    }
         
    if (blocks_to_delay == 0) {
        accumulated_fractional_ticks += nominator;
        if (accumulated_fractional_ticks < denominator) {                
            set_timer_cycles(tail_ticks);
        } else {
            set_timer_cycles(tail_ticks+1);
            accumulated_fractional_ticks -= denominator;
        }              
    }     
}
 
TRICK17(void set_target_frequency(const fixed_point target_frequency)) {
 
    // total delay time will be delay_block_tick_size * delay_blocks + tail_ticks + nominator/denominator  
    cli();
 
    // compute the integer part of the period length "delay_ticks" as well as its fractional part "nominator/denominator"
    denominator = target_frequency * phases;       
    uint64_t delay_ticks = system_clock_frequency / denominator;    
    nominator = system_clock_frequency - delay_ticks * denominator;
  
    // break down delay_ticks in chunks that can be handled with a 16 bit timer   
    delay_blocks = delay_ticks / delay_block_tick_size;
    tail_ticks   = delay_ticks - delay_block_tick_size * delay_blocks;
    if (delay_blocks > 0) {
        // enforce that tails are always longer than 1000 ticks
        --delay_blocks;
        tail_ticks += delay_block_tick_size;
    }
         
    // tell the timer ISR to reset its internal values
    reset_timer = true;
 
    sei();    
     
    // attention: the target frequency is fixed_point but output is uint32_t
    Serial.print(F("target frequency: ")); Serial.println(target_frequency); 
    /*
    // debugging only
    Serial.print(F("denominator: ")); Serial.println(denominator); 
    Serial.print(F("nominator: ")); Serial.println(nominator); 
    Serial.print(F("delay ticks: ")); Serial.print((uint32_t)delay_ticks); 
    Serial.print(','); Serial.println((uint32_t)(delay_ticks>>32)); 
    Serial.print(F("delay blocks: ")); Serial.println(delay_blocks);     
    Serial.print(F("tail ticks: ")); Serial.println(tail_ticks);     
    /**/
};
 
 
void setup() {
    DDRD = 0b11111111; // set digital  0- 7 to output
    DDRB = 0b00111111; // set digital  8-13 to output
    DDRC = 0b00111111; // set digital 14-19 to output (coincidences with analog 0-5)
 
   Serial.begin(115200);
 
    // disable timer0 interrupts to stop Arduino timer functions
    // from introducing jitter in our output
    TIMSK0 = 0;
 
    // disable timer1 interrupts
    TIMSK1 = 0;
 
    // do not toggle or change timer IO pins
    TCCR1A = 0;
    // Mode 4, CTC using OCR1A | set prescaler to 1
    TCCR1B = (1<<WGM12) | (1<<CS10);
 
    set_target_frequency(default_frequency);
 
    // enable match interrupts
    TIMSK1 = 1<<OCIE1A;    
     
}
 
void loop() { 
    set_target_frequency(parse());    
}
