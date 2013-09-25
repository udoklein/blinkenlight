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
 
 
const uint8_t dcf77_sample_pin = 19; // A5
const uint8_t dcf77_analog_sample_pin = 5;
const uint8_t dcf77_monitor_pin = 18;
const uint8_t dcf77_synthetic_clock_pin = 17;
 
const uint8_t first_debug_pin = 5;
 
 
namespace DCF77_Demodulator {
    const uint8_t bin_count = 100;
     
    typedef struct {
        uint16_t data[bin_count];
        uint8_t tick;
         
        uint32_t noise_max;
        uint32_t max;
        uint8_t max_index;
    } phase_bins;
     
    phase_bins bins;
     
    const uint16_t samples_per_second = 1000;
     
    const uint16_t samples_per_bin = samples_per_second / bin_count;
    const uint16_t bins_per_10ms  = bin_count / 100;
    const uint16_t bins_per_50ms  =  5 * bins_per_10ms;
    const uint16_t bins_per_60ms  =  6 * bins_per_10ms;
    const uint16_t bins_per_100ms = 10 * bins_per_10ms;
    const uint16_t bins_per_200ms = 20 * bins_per_10ms;
    const uint16_t bins_per_500ms = 50 * bins_per_10ms;
 
     
    void setup() {
        for (uint8_t index = 0; index < bin_count; ++index) {
            bins.data[index] = 0;
        }
        bins.tick = 0;
         
        bins.max = 0;
        bins.max_index = 255;
        bins.noise_max = 0;
    }
 
    uint16_t wrap(const uint16_t value) {
        // faster modulo function which avoids division
        uint16_t result = value;
        while (result >= bin_count) {
            result-= bin_count;
        }
        return result;
    }
     
    void phase_detection() {
        // We will compute the integrals over 200ms.
        // The integrals is used to find the window of maximum signal strength.
        uint32_t integral = 0;
         
        for (uint16_t bin = 0; bin < bins_per_100ms; ++bin)  {
            integral += ((uint32_t)bins.data[bin])<<1;
        }
         
        for (uint16_t bin = bins_per_100ms; bin < bins_per_200ms; ++bin)  {
            integral += (uint32_t)bins.data[bin];
        }
         
        bins.max = 0;
        bins.max_index = 0;
        for (uint16_t bin = 0; bin < bin_count; ++bin) {
            if (integral > bins.max) {
                bins.max = integral;
                bins.max_index = bin;
            }
             
            integral -= (uint32_t)bins.data[bin]<<1;
            integral += (uint32_t)(bins.data[wrap(bin + bins_per_100ms)] + bins.data[wrap(bin + bins_per_200ms)]);
        }
         
        // max_index indicates the position of the 200ms second signal window.
        // Now how can we estimate the noise level? This is very tricky because
        // averaging has already happened to some extend.
         
        // The issue is that most of the undesired noise happens around the signal,
        // especially after high->low transitions. So as an approximation of the
        // noise I test with a phase shift of 200ms.
        bins.noise_max = 0;
        const uint16_t noise_index = wrap(bins.max_index + bins_per_200ms);
         
        for (uint16_t bin = 0; bin < bins_per_100ms; ++bin)  {
            bins.noise_max += ((uint32_t)bins.data[wrap(noise_index + bin)])<<1;
        }
         
        for (uint16_t bin = bins_per_100ms; bin < bins_per_200ms; ++bin)  {
            bins.noise_max += (uint32_t)bins.data[wrap(noise_index + bin)];
        }
    }
 
    void advance_tick() {
        if (bins.tick < bin_count - 1) {
            ++bins.tick;
        } else {
            bins.tick = 0;
        }
    }
     
    uint8_t phase_binning(const uint8_t input) {
        // how many seconds may be cummulated
        // this controls how slow the filter may be to follow a phase drift
        // N times the clock precision shall be smaller 1
        // clock 30 ppm => N < 300
        const uint16_t N = 300;
         
        advance_tick();
         
        if (input) {
            if (bins.data[bins.tick] < N) {
                ++bins.data[bins.tick];
            }
        } else {
            if (bins.data[bins.tick] > 0) {
                --bins.data[bins.tick];
            }
        }
        return bins.tick;
    }
     
    void detector_stage_2(const uint8_t input) {
        const uint8_t current_bin = bins.tick;
         
        const uint8_t threshold = 30;
         
        if (bins.max-bins.noise_max < threshold ||
            wrap(bin_count + current_bin - bins.max_index) == 53) {
            // Phase detection far enough out of phase from anything that
            // might consume runtime otherwise.
            phase_detection();
        }
    }
     
    void detector(const uint8_t sampled_data) {
        static uint8_t current_sample = 0;
        static uint8_t average = 0;
         
        // detector stage 0: average 10 samples (per bin)
        average += sampled_data;
         
        if (++current_sample >= samples_per_bin) {
            // once all samples for the current bin are captured the bin gets updated
            // that is each 10ms control is passed to stage 1
            const uint8_t input = (average> samples_per_bin/2);
             
            phase_binning(input);
             
            detector_stage_2(input);
             
            average = 0;
            current_sample = 0;
        }
    }
     
    void get_quality(uint32_t &lock_max, uint32_t &noise_max) {
        lock_max = bins.max;
        noise_max = bins.noise_max;
    }
 
    uint8_t get_time_value() {
        const uint8_t threshold = 2;
         
        return (bins.max-bins.noise_max >= threshold)? (bins.max_index + bins.tick + 1) % bin_count: -1;
    }
     
    uint8_t debug() {
        Serial.print(F("Phase: "));
         
        const bool uses_integrals = sizeof(bins.max) == 4;
         
        Serial.print(get_time_value(), HEX);
        Serial.print(F(" Tick: "));
        Serial.print(bins.tick);
        Serial.print(F(" Quality: "));
        Serial.print(bins.max, DEC);
        Serial.print('-');
        Serial.print(bins.noise_max, DEC);
        Serial.print(F(" Max Index: "));
        Serial.print(bins.max_index, DEC);
        Serial.print('>');
         
        for (uint8_t index = 0; index < bin_count; ++index) {
            if (index == bins.max_index ||
                (!uses_integrals && index == (bins.max_index+1) % bin_count) ||
                (uses_integrals && (index == (bins.max_index+10) % bin_count ||
                (index == (bins.max_index+20) % bin_count)))) {
                Serial.print('|');
                }
                Serial.print(bins.data[index],HEX);
        }
        Serial.println();
        return bins.max_index;
    }
}
 
 
void process_one_sample() {
    // The Blinkenlight LEDs will cut off the input signal
    // below a logical high. This is due to the weak
    // output signal of the DCF module.
    // Hence for the Blinkenlighty
    // and the Blinkenlight shield we can not use
    // digitalRead.
     
    // Comment the line below if you are not using this code with a
    // Blinkenlighty or Blinkenlight shield.
    const uint8_t sampled_data = analogRead(dcf77_analog_sample_pin)>200? 1: 0;
    // Uncomment the line below if you are using this code with a standard Arduino
    //const uint8_t sampled_data = digitalRead(dcf77_sample_pin);
         
    digitalWrite(dcf77_monitor_pin, sampled_data);
     
    DCF77_Demodulator::detector(sampled_data);
}
 
ISR(TIMER2_COMPA_vect) {
    process_one_sample();
}
 
void initTimer2() {
    // Timer 2 CTC mode, prescaler 64
    TCCR2B = (1<<WGM22) | (1<<CS22);
    TCCR2A = (1<<WGM21);
     
    // 249 + 1 == 250 == 250 000 / 1000 =  (16 000 000 / 64) / 1000
    OCR2A = 249;
     
    // enable Timer 2 interrupts
    TIMSK2 = (1<<OCIE2A);
}
 
void stopTimer0() {
    // ensure that the standard timer interrupts will not
    // mess with msTimer2
    TIMSK0 = 0;
}
 
 
void setup() {
    Serial.begin(115200);
    Serial.println();
 
    pinMode(dcf77_sample_pin, INPUT);
    digitalWrite(dcf77_sample_pin, HIGH);
 
    pinMode(dcf77_monitor_pin, OUTPUT);
    pinMode(dcf77_synthetic_clock_pin, OUTPUT);
     
    for (uint8_t pin = 0; pin<10; ++pin) {
        pinMode(first_debug_pin + pin, OUTPUT);
    }
 
    DCF77_Demodulator::setup();
 
    initTimer2();
    stopTimer0();
}
 
void loop() {
    using namespace DCF77_Demodulator;
     
    const uint8_t phase = debug();
     
    for (uint8_t pin = 0; pin<10; ++pin) {
        digitalWrite(first_debug_pin + pin, phase % 10 == pin);
    }
    digitalWrite(dcf77_synthetic_clock_pin, (bins.tick - phase + bin_count) % bin_count < bin_count / 10);
}
