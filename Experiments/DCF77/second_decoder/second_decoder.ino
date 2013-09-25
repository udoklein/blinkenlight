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
 
 
// signatures and types --> will go into a separate .h file later on
namespace Hamming {
    typedef struct {
        uint8_t lock_max;
        uint8_t noise_max;
    } lock_quality_t;
}
 
namespace DCF77 {
    const uint8_t long_tick  = 3;
    const uint8_t short_tick = 2;
    const uint8_t undefined  = 1;
    const uint8_t sync_mark  = 0;
     
    typedef struct {
        uint8_t second;      // 0..60
    } time_data;
}
 
namespace DCF77_Second_Decoder {
    void setup();
    void process_single_tick_data(const uint8_t tick_data);
    uint8_t get_second();
    void get_quality(Hamming::lock_quality_t &lock_quality);
     
    void debug();
}
 
namespace DCF77_Clock_Controller {
    void setup();
    void process_single_tick_data(const uint8_t tick_data);
     
    void flush();
    typedef void (*flush_handler)(const DCF77::time_data &decoded_time);
     
    void default_handler(const DCF77::time_data &decoded_time);
    void set_flush_handler(const flush_handler output_handler);
     
    typedef Hamming::lock_quality_t lock_quality_t;
     
    typedef struct {
        struct {
            uint32_t lock_max;
            uint32_t noise_max;
        } phase;
         
        lock_quality_t second;
    } clock_quality_t;
     
    void get_quality(clock_quality_t &clock_quality);
     
    // blocking, will unblock at the start of the second
    void get_current_time(DCF77::time_data &now);
}
 
namespace DCF77_Demodulator {
    void setup();
    void detector(const uint8_t sampled_data);
    void get_quality(uint32_t &lock_max, uint32_t &noise_max);
    void get_noise_indicator(uint32_t &noise_indicator);
     
    void debug();
}
 
 
// library implementation --> will go into a separate .cpp file later on
namespace Arithmetic_Tools {
    template <uint8_t N> inline void bounded_increment(uint8_t &value) __attribute__((always_inline));
    template <uint8_t N>
    void bounded_increment(uint8_t &value) {
        if (value >= 255 - N) { value = 255; } else { value += N; }
    }
     
    template <uint8_t N> inline void bounded_decrement(uint8_t &value) __attribute__((always_inline));
    template <uint8_t N>
    void bounded_decrement(uint8_t &value) {
        if (value <= N) { value = 0; } else { value -= N; }
    }
}
 
namespace Hamming {
    template <typename bins_t>
    void advance_tick(bins_t &bins) {
        const uint8_t number_of_bins = sizeof(bins.data) / sizeof(bins.data[0]);
        if (bins.tick < number_of_bins - 1) {
            ++bins.tick;
        } else {
            bins.tick = 0;
        }
    }
     
    template <typename bins_t>
    void compute_max_index(bins_t &bins) {
        const uint8_t number_of_bins = sizeof(bins.data) / sizeof(bins.data[0]);
         
        bins.noise_max = 0;
        bins.max = 0;
        bins.max_index = 255;
        for (uint8_t index = 0; index < number_of_bins; ++index) {
            const uint8_t bin_data = bins.data[index];
             
            if (bin_data >= bins.max) {
                bins.noise_max = bins.max;
                bins.max = bin_data;
                bins.max_index = index;
            } else if (bin_data > bins.noise_max) {
                bins.noise_max = bin_data;
            }
        }
    }
     
    template <typename bins_t>
    void setup(bins_t &bins) {
        const uint8_t number_of_bins = sizeof(bins.data) / sizeof(bins.data[0]);
         
        for (uint8_t index = 0; index < number_of_bins; ++index) {
            bins.data[index] = 0;
        }
        bins.tick = 0;
         
        bins.max = 0;
        bins.max_index = 255;
        bins.noise_max = 0;
    }
     
    template <typename bins_t>
    uint8_t get_time_value(const bins_t &bins) {
        // there is a trade off involved here:
        //    low threshold --> lock will be detected earlier
        //    low threshold --> if lock is not clean output will be garbled
        //    a proper lock will fix the issue
        //    the question is: which start up behaviour do we prefer?
        const uint8_t threshold = 2;
         
        const uint8_t number_of_bins = sizeof(bins.data) / sizeof(bins.data[0]);
         
        if (bins.max-bins.noise_max >= threshold) {
            return (bins.max_index + bins.tick + 1) % number_of_bins;
        } else {
            const uint8_t undefined = 0xff;
            return undefined;
        }
    }
     
    template <typename bins_t>
    void get_quality(const bins_t bins, Hamming::lock_quality_t &lock_quality) {
        lock_quality.lock_max = bins.max;
        lock_quality.noise_max = bins.noise_max;
    }
     
    template <typename bins_t>
    void debug (const bins_t &bins) {
        const uint8_t number_of_bins = sizeof(bins.data) / sizeof(bins.data[0]);
        const bool uses_integrals = sizeof(bins.max) == 4;
         
        Serial.print(get_time_value(bins), HEX);
        Serial.print(F(" Tick: "));
        Serial.print(bins.tick);
        Serial.print(F(" Quality: "));
        Serial.print(bins.max, DEC);
        Serial.print('-');
        Serial.print(bins.noise_max, DEC);
        Serial.print(F(" Max Index: "));
        Serial.print(bins.max_index, DEC);
        Serial.print('>');
         
        for (uint8_t index = 0; index < number_of_bins; ++index) {
            if (index == bins.max_index ||
                (!uses_integrals && index == (bins.max_index+1) % number_of_bins) ||
                (uses_integrals && (index == (bins.max_index+10) % number_of_bins ||
                (index == (bins.max_index+20) % number_of_bins)))) {
                Serial.print('|');
                }
                Serial.print(bins.data[index],HEX);
        }
        Serial.println();
    }
}
 
namespace DCF77_Second_Decoder {
    using namespace DCF77;
     
    const uint8_t seconds_per_minute = 60;
    // this is a trick threshold
    //    lower it to get a faster second lock
    //    but then risk to garble the successive stages during startup
    //    --> to low and total startup time will increase
    const uint8_t lock_threshold = 12;
     
    typedef struct {
        uint8_t data[seconds_per_minute];
        uint8_t tick;
         
        uint8_t noise_max;
        uint8_t max;
        uint8_t max_index;
    } sync_bins;
     
    sync_bins bins;
     
    void sync_mark_binning(const uint8_t tick_data) {
        // We use a binning approach to find out the proper phase.
        // The goal is to localize the sync_mark. Due to noise
        // there may be wrong marks of course. The idea is to not
        // only look at the statistics of the marks but to exploit
        // additional data properties:
         
        // Bit position  0 after a proper sync is a 0.
        // Bit position 20 after a proper sync is a 1.
         
        // The binning will work as follows:
         
        //   1) A sync mark will score +6 points for the current bin
        //      it will also score -2 points for the previous bin
        //                         -2 points for the following bin
        //                     and -2 points 20 bins later
        //  In total this will ensure that a completely lost signal
        //  will not alter the buffer state (on average)
         
        //   2) A 0 will score +1 point for the previous bin
        //      it also scores -2 point 20 bins back
        //                 and -2 points for the current bin
         
        //   3) A 1 will score +1 point 20 bins back
        //      it will also score -2 point for the previous bin
        //                     and -2 points for the current bin
         
        //   4) An undefined value will score -2 point for the current bin
        //                                    -2 point for the previous bin
        //                                    -2 point 20 bins back
         
        //   5) Scores have an upper limit of 255 and a lower limit of 0.
         
        // Summary: sync mark earns 6 points, a 0 in position 0 and a 1 in position 20 earn 1 bonus point
        //          anything that allows to infer that any of the "connected" positions is not a sync will remove 2 points
         
        // It follows that the score of a sync mark (during good reception)
        // may move up/down the whole scale in slightly below 64 minutes.
        // If the receiver should glitch for whatever reason this implies
        // that the clock will take about 33 minutes to recover the proper
        // phase (during phases of good reception). During bad reception things
        // are more tricky.
        using namespace Arithmetic_Tools;
         
        const uint8_t previous_tick = bins.tick>0? bins.tick-1: seconds_per_minute-1;
        const uint8_t previous_21_tick = bins.tick>20? bins.tick-21: bins.tick + seconds_per_minute-21;
         
        switch (tick_data) {
            case sync_mark:
                bounded_increment<6>(bins.data[bins.tick]);
                 
                bounded_decrement<2>(bins.data[previous_tick]);
                bounded_decrement<2>(bins.data[previous_21_tick]);
                 
                { const uint8_t next_tick = bins.tick< seconds_per_minute-1? bins.tick+1: 0;
                bounded_decrement<2>(bins.data[next_tick]); }
                break;
                 
            case short_tick:
                bounded_increment<1>(bins.data[previous_tick]);
                 
                bounded_decrement<2>(bins.data[bins.tick]);
                bounded_decrement<2>(bins.data[previous_21_tick]);
                break;
                 
            case long_tick:
                bounded_increment<1>(bins.data[previous_21_tick]);
                 
                bounded_decrement<2>(bins.data[bins.tick]);
                bounded_decrement<2>(bins.data[previous_tick]);
                break;
                 
            case undefined:
            default:
                bounded_decrement<2>(bins.data[bins.tick]);
                bounded_decrement<2>(bins.data[previous_tick]);
                bounded_decrement<2>(bins.data[previous_21_tick]);
        }
        bins.tick = bins.tick<seconds_per_minute-1? bins.tick+1: 0;
         
        // determine sync lock
        if (bins.max - bins.noise_max <=lock_threshold ||
            get_second() == 3) {
            // after a lock is acquired this happens only once per minute and it is
            // reasonable cheap to process,
            //
            // that is: after we have a "lock" this will be processed whenever
            // the sync mark was detected
             
            Hamming::compute_max_index(bins);
            }
    }
     
    void get_quality(Hamming::lock_quality_t &lock_quality) {
        Hamming::get_quality(bins, lock_quality);
    }
     
    uint8_t get_second() {
        if (bins.max - bins.noise_max >= lock_threshold) {
            // at least one sync mark and a 0 and a 1 seen
            // the threshold is tricky:
            //   higher --> takes longer to acquire an initial lock, but higher probability of an accurate lock
            //
            //   lower  --> higher probability that the lock will oscillate at the beginning
            //              and thus spoil the downstream stages
             
            // we have to subtract 2 seconds
            //   1 because the seconds already advanced by 1 tick
            //   1 because the sync mark is not second 0 but second 59
             
            uint8_t second = 2*seconds_per_minute + bins.tick - 2 - bins.max_index;
            while (second >= seconds_per_minute) { second-= seconds_per_minute; }
             
            return second;
        } else {
            return 0xff;
        }
    }
     
    void process_single_tick_data(const uint8_t tick_data) {
        sync_mark_binning(tick_data);
    }
     
    void setup() {
        Hamming::setup(bins);
    }
     
    void debug() {
        static uint8_t prev_tick;
         
        if (prev_tick == bins.tick) {
            return;
        } else {
            prev_tick = bins.tick;
             
            Serial.print(F("second: "));
            Serial.print(get_second(), DEC);
            Serial.print(F(" Sync mark index "));
            Hamming::debug(bins);
             
            Serial.println();
        }
    }
}
 
namespace DCF77_Clock_Controller {
    flush_handler output_handler = 0;
     
    DCF77::time_data decoded_time;
    volatile bool second_toggle;
     
    void get_current_time(DCF77::time_data &now) {
        for (bool stopper = second_toggle; stopper == second_toggle; ) {
            // wait for second_toggle to toggle
            // that is wait for decoded time to be ready
        }
        now = decoded_time;
    }
     
    void flush() {
        // this is called at the end of each second / before the next second begins
        // it is most interesting to propagate this further at the end of a sync marks
        // it is also interesting to propagate this to reference clocks
         
        decoded_time.second = DCF77_Second_Decoder::get_second();
        second_toggle = !second_toggle;
         
        // decoded_time holds the value of the current second but
        // we are immediately before the start of the next
        // second --> add 1 second to decoded_time
         
        if (decoded_time.second < 60) {
            decoded_time.second = (decoded_time.second + 1) % 60;
        }
         
        if (output_handler) {
            output_handler(decoded_time);
        }
    }
     
    void set_flush_handler(const flush_handler new_output_handler) {
        output_handler = new_output_handler;
    }
     
    void get_quality(clock_quality_t &clock_quality) {
        DCF77_Demodulator::get_quality(clock_quality.phase.lock_max, clock_quality.phase.noise_max);
        DCF77_Second_Decoder::get_quality(clock_quality.second);
    }
     
    void setup() {
        DCF77_Second_Decoder::setup();
    }
     
    void process_single_tick_data(const uint8_t tick_data) {
        DCF77_Second_Decoder::process_single_tick_data(tick_data);
    }
}
 
namespace DCF77_Demodulator {
    using namespace DCF77;
     
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
        Hamming::setup(bins);
         
        DCF77_Clock_Controller::setup();
    }
     
    void decode_220ms(const uint8_t input, const uint8_t bins_to_go) {
        // will be called for each bin during the "interesting" 220ms
         
        static uint8_t count = 0;
        static uint8_t decoded_data = 0;
         
        count += input;
        if (bins_to_go >= bins_per_100ms + bins_per_10ms) {
            if (bins_to_go == bins_per_100ms + bins_per_10ms) {
                decoded_data = count > bins_per_50ms? 2: 0;
                count = 0;
            }
        } else {
            if (bins_to_go == 0) {
                decoded_data += count > bins_per_50ms? 1: 0;
                count = 0;
                // pass control further
                // decoded_data: 3 --> 1
                //               2 --> 0,
                //               1 --> undefined,
                //               0 --> sync_mark
                DCF77_Clock_Controller::process_single_tick_data(decoded_data);
            }
        }
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
     
    uint8_t phase_binning(const uint8_t input) {
        // how many seconds may be cummulated
        // this controls how slow the filter may be to follow a phase drift
        // N times the clock precision shall be smaller 1
        // clock 30 ppm => N < 300
        const uint16_t N = 300;
         
        Hamming::advance_tick(bins);
         
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
             
            static uint8_t bins_to_process = 0;
         
        if (bins_to_process == 0) {
            if (wrap((bin_count + current_bin - bins.max_index)) <= bins_per_100ms ||   // current_bin at most 100ms after phase_bin
                wrap((bin_count + bins.max_index - current_bin)) <= bins_per_10ms ) {   // current bin at most 10ms before phase_bin
                // if phase bin varies to much during one period we will always be screwed in may ways...
                 
                // last 10ms of current second
                DCF77_Clock_Controller::flush();
             
            // start processing of bins
                bins_to_process = bins_per_200ms + 2*bins_per_10ms;
                }
        }
         
        if (bins_to_process > 0) {
            --bins_to_process;
             
            // this will be called for each bin in the "interesting" 220ms
            // this is also a good place for a "monitoring hook"
            decode_220ms(input, bins_to_process);
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
     
    void debug() {
        Serial.print(F("Phase: "));
        Hamming::debug(bins);
    }
}
 
 
// main program starts here
 
const uint8_t dcf77_sample_pin = 19; // A5
const uint8_t dcf77_analog_sample_pin = 5;
const uint8_t dcf77_monitor_pin = 18;
 
const uint8_t first_output_pin = 5;
 
 
void process_one_sample() {
    // The Blinkenlight LEDes will cut off the input signal
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
 
void output_handler(const DCF77::time_data &decoded_time) {
    uint8_t lo;
    uint8_t hi;
    uint8_t out;
 
    if (decoded_time.second < 60) {
        // bcd conversion
        hi = decoded_time.second / 10;
        lo = decoded_time.second - 10 * hi;
        out = (hi<<4) + lo;
 
        uint8_t pin = first_output_pin;
        for (uint8_t bits=0; bits<8; ++bits) {
            digitalWrite(pin++, out & 0x1);
            out >>= 1;
        }
    }
}
 
void setup() {
    Serial.begin(115200);
    Serial.println();
 
    pinMode(dcf77_monitor_pin, OUTPUT);
 
    pinMode(dcf77_sample_pin, INPUT);
    digitalWrite(dcf77_sample_pin, HIGH);
 
    for (uint8_t pin=first_output_pin; pin<first_output_pin+8; ++pin) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
    }
 
    stopTimer0();
    initTimer2();
    DCF77_Demodulator::setup();
    DCF77_Clock_Controller::set_flush_handler(&output_handler);
     
//    Serial.println(F("acquiring lock - please wait"));
}
 
void loop() {
/*
    DCF77::time_data now;
     
    DCF77_Clock_Controller::get_current_time(now);
    if (now.second <= 60) {
        Serial.println();
        Serial.print(F("Decoded time: "));
        Serial.println(now.second);
    }
*/
 
    //Serial.println();
    //DCF77_Demodulator::debug();
    DCF77_Second_Decoder::debug();
}
