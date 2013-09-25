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
 
 
//#define DEBUG 1
 
 
const uint8_t first_output_pin = 5;
 
namespace Debug {
    void debug_helper(char data);
    void bcddigit(uint8_t data);
    void bcddigits(uint8_t data);
}
 
namespace Hamming {
    typedef struct {
        uint8_t lock_max;
        uint8_t noise_max;
    } lock_quality_t;
}
 
namespace BCD {
    typedef union {
        struct {
            uint8_t lo:4;
            uint8_t hi:4;
        } digit;
 
        struct {
            uint8_t b0:1;
            uint8_t b1:1;
            uint8_t b2:1;
            uint8_t b3:1;
            uint8_t b4:1;
            uint8_t b5:1;
            uint8_t b6:1;
            uint8_t b7:1;
        } bit;
 
        uint8_t val;
    } bcd_t;
 
    void increment(bcd_t &value);
 
    bcd_t int_to_bcd(const uint8_t value);
    uint8_t bcd_to_int(const uint8_t value);
}
 
typedef union {
    struct {
        uint8_t b0:2;
        uint8_t b1:2;
        uint8_t b2:2;
        uint8_t b3:2;
    } signal;
    uint8_t byte;
} tData;
 
namespace DCF77 {
    const uint8_t long_tick  = 3;
    const uint8_t short_tick = 2;
    const uint8_t undefined  = 1;
    const uint8_t sync_mark  = 0;
 
    typedef struct {
        BCD::bcd_t hour;     // 0..23
        BCD::bcd_t minute;   // 0..59
        uint8_t second;      // 0..60
 
        boolean undefined_minute_output;
    } time_data;
}
 
namespace DCF77_Encoder {
 
    void reset(DCF77::time_data &now);
    void advance_second(DCF77::time_data &now);
 
    uint8_t get_current_signal(const DCF77::time_data &now);
 
    void debug(const DCF77::time_data &clock);
    void debug(const DCF77::time_data &clock, const uint16_t cycles);
 
    // Bit      Bezeichnung     Wert    Pegel   Bedeutung
    // 0        M                       0       Minutenanfang (
 
    // 1..14    n/a                             reserviert
 
    // 15       R                               Reserveantenne aktiv (0 inaktiv, 1 aktiv)
    // 16       A1                              Ankündigung Zeitzonenwechsel (1 Stunde vor dem Wechsel für 1 Stunde, d.h ab Minute 1)
    // 17       Z1               2              Zeitzonenbit Sommerzeit (MEZ = 0, MESZ = 1); also Zeitzone = UTC + 2*Z1 + Z2
    // 18       Z2               1              Zeitzonenbit Winterzeit (MEZ = 1, MESZ = 0); also Zeitzone = UTC + 2*Z1 + Z2
    // 19       A2                              Ankündigung einer Schaltsekunde (1 Stunde vor der Schaltsekunde für 1 Stunde, d.h. ab Minute 1)
 
    // 20       S                       1       Startbit für Zeitinformation
 
    // 21                        1              Minuten  1er
    // 22                        2              Minuten  2er
    // 23                        4              Minuten  4er
    // 24                        8              Minuten  8er
    // 25                       10              Minuten 10er
    // 26                       20              Minuten 20er
    // 27                       40              Minuten 40er
    // 28       P1                              Prüfbit 1 (gerade Parität)
 
    // 29                        1              Stunden  1er
    // 30                        2              Stunden  2er
    // 31                        4              Stunden  4er
    // 32                        8              Stunden  8er
    // 33                       10              Stunden 10er
    // 34                       20              Stunden 20er
    // 35       P2                              Prüfbit 2 (gerade Parität)
 
    // 36                        1              Tag  1er
    // 37                        2              Tag  2er
    // 38                        4              Tag  4er
    // 39                        8              Tag  8er
    // 40                       10              Tag 10er
    // 41                       20              Tag 20er
 
    // 42                        1              Wochentag 1er (Mo = 1, Di = 2, Mi = 3,
    // 43                        2              Wochentag 2er (Do = 4, Fr = 5, Sa = 6,
    // 44                        4              Wochentag 4er (So = 7)
 
    // 45                        1              Monat  1er
    // 46                        2              Monat  2er
    // 47                        4              Monat  4er
    // 48                        8              Monat  8er
    // 49                       10              Monat 10er
 
    // 50                        1              Jahr  1er
    // 51                        2              Jahr  2er
    // 52                        4              Jahr  4er
    // 53                        8              Jahr  8er
    // 54                       10              Jahr 10er
    // 55                       20              Jahr 20er
    // 56                       40              Jahr 40er
    // 57                       80              Jahr 80er
 
    // 58       P3                              Prüftbit 3 (gerade Parität)
 
    // 59       sync                            Sync Marke, kein Impuls (übliches Minutenende)
    // 59                               0       Schaltsekunde (sehr selten, nur nach Ankündigung)
    // 60       sync                            Sync Marke, kein Impuls (nur nach Schaltsekunde)
 
    // Falls eine Schaltsekunde eingefügt wird, wird bei Bit 59 eine Sekundenmarke gesendet.
    // Der Syncimpuls erfolgt dann in Sekunde 60 statt 59. Üblicherweise wird eine 0 als Bit 59 gesendet
 
    // Üblicherweise springt die Uhr beim Wechsel Winterzeit nach Sommerzeit von 1:59:59 auf 3:00:00
    //                               beim Wechsel Sommerzeit nach Winterzeit von 2:59:59 auf 2:00:00
 
    // Die Zeitinformation wird immer 1 Minute im Vorraus übertragen. D.h. nach der Syncmarke hat
    // man die aktuelle Zeit
 
    // http://www.dcf77logs.de/SpecialFiles.aspx
 
    // Schaltsekunden werden in Deutschland von der Physikalisch-Technischen Bundesanstalt festgelegt,
    // die allerdings dazu nur die international vom International Earth Rotation and Reference Systems
    // Service (IERS) festgelegten Schaltsekunden übernimmt. Im Mittel sind Schaltsekunden etwa alle 18
    // Monate nötig und werden vorrangig am 31. Dezember oder 30. Juni, nachrangig am 31. März oder
    // 30. September nach 23:59:59 UTC (also vor 1:00 MEZ bzw. 2:00 MESZ) eingefügt. Seit der Einführung
    // des Systems 1972 wurden ausschließlich die Zeitpunkte im Dezember und Juni benutzt.
}
 
namespace DCF77_Hour_Decoder {
    void setup();
    void process_tick(const uint8_t current_second, const uint8_t tick_value);
    void advance_hour();
    BCD::bcd_t get_hour();
    void get_quality(Hamming::lock_quality_t &lock_quality);
 
    void debug();
}
 
namespace DCF77_Minute_Decoder {
    void setup();
    void process_tick(const uint8_t current_second, const uint8_t tick_value);
    void advance_minute();
    BCD::bcd_t get_minute();
 
    void debug();
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
        lock_quality_t minute;
        lock_quality_t hour;
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
 
namespace Debug {
    void debug_helper(char data) { Serial.print(data == 0? 'S': data == 1? '?': data - 2 + '0', 0); }
 
    void bcddigit(uint8_t data) {
        if (data <= 0x09) {
            Serial.print(data, HEX);
        } else {
            Serial.print('?');
        }
    }
 
    void bcddigits(uint8_t data) {
        bcddigit(data >>  4);
        bcddigit(data & 0xf);
    }
}
 
namespace BCD {
    void increment(bcd_t &value) {
        if (value.digit.lo < 9) {
            ++value.digit.lo;
        } else {
            value.digit.lo = 0;
 
            if (value.digit.hi < 9) {
                ++value.digit.hi;
            } else {
                value.digit.hi = 0;
            }
        }
    }
 
    bcd_t int_to_bcd(const uint8_t value) {
        const uint8_t hi = value / 10;
 
        bcd_t result;
        result.digit.hi = hi;
        result.digit.lo = value-10*hi;
 
        return result;
    }
 
    uint8_t bcd_to_int(const bcd_t value) {
        return value.digit.lo + 10*value.digit.hi;
    }
}
 
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
 
    inline void bounded_add(uint8_t &value, const uint8_t amount) __attribute__((always_inline));
    void bounded_add(uint8_t &value, const uint8_t amount) {
        if (value >= 255-amount) { value = 255; } else { value += amount; }
    }
 
    inline void bounded_sub(uint8_t &value, const uint8_t amount) __attribute__((always_inline));
    void bounded_sub(uint8_t &value, const uint8_t amount) {
        if (value <= amount) { value = 0; } else { value -= amount; }
    }
 
    inline uint8_t bit_count(const uint8_t value) __attribute__((always_inline));
    uint8_t bit_count(const uint8_t value) {
        const uint8_t tmp1 = (value & 0b01010101) + ((value>>1) & 0b01010101);
        const uint8_t tmp2 = (tmp1  & 0b00110011) + ((tmp1>>2) & 0b00110011);
        return (tmp2 & 0x0f) + (tmp2>>4);
    }
 
    inline uint8_t parity(const uint8_t value) __attribute__((always_inline));
    uint8_t parity(const uint8_t value) {
        uint8_t tmp = value;
 
        tmp = (tmp & 0xf) ^ (tmp >> 4);
        tmp = (tmp & 0x3) ^ (tmp >> 2);
        tmp = (tmp & 0x1) ^ (tmp >> 1);
 
        return tmp;
    }
}
 
namespace Hamming {
    template <uint8_t significant_bits>
    void score (uint8_t &bin, const BCD::bcd_t input, const BCD::bcd_t candidate) {
        using namespace Arithmetic_Tools;
 
        const uint8_t the_score = significant_bits - bit_count(input.val ^ candidate.val);
        bounded_add(bin, the_score);
    }
 
    template <typename bins_t>
    void advance_tick(bins_t &bins) {
        const uint8_t number_of_bins = sizeof(bins.data) / sizeof(bins.data[0]);
        if (bins.tick < number_of_bins - 1) {
            ++bins.tick;
        } else {
            bins.tick = 0;
        }
    }
 
    template <typename bins_type, uint8_t significant_bits, bool with_parity>
    void hamming_binning(bins_type &bins, const BCD::bcd_t input) {
        using namespace Arithmetic_Tools;
        using namespace BCD;
 
        const uint8_t number_of_bins = sizeof(bins.data) / sizeof(bins.data[0]);
 
        if (bins.max > 255-significant_bits) {
            // If we know we can not raise the maximum any further we
            // will lower the noise floor instead.
            for (uint8_t bin_index = 0; bin_index <number_of_bins; ++bin_index) {
                bounded_decrement<significant_bits>(bins.data[bin_index]);
            }
            bins.max -= significant_bits;
            bounded_decrement<significant_bits>(bins.noise_max);
        }
 
        const uint8_t offset = number_of_bins-1-bins.tick;
        uint8_t bin_index = offset;
 
        bcd_t candidate;
        candidate.val = 0x00;
        for (uint8_t pass=0; pass < number_of_bins; ++pass) {
 
            if (with_parity) {
                candidate.bit.b7 = parity(candidate.val);
                score<significant_bits>(bins.data[bin_index], input, candidate);
                candidate.bit.b7 = 0;
            } else {
                score<significant_bits>(bins.data[bin_index], input, candidate);
            }
 
            bin_index = bin_index < number_of_bins-1? bin_index+1: 0;
            increment(candidate);
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
    BCD::bcd_t get_time_value(const bins_t &bins) {
        // there is a trade off involved here:
        //    low threshold --> lock will be detected earlier
        //    low threshold --> if lock is not clean output will be garbled
        //    a proper lock will fix the issue
        //    the question is: which start up behaviour do we prefer?
        const uint8_t threshold = 2;
 
        const uint8_t number_of_bins = sizeof(bins.data) / sizeof(bins.data[0]);
        const uint8_t offset = (number_of_bins == 60 || number_of_bins == 24 || number_of_bins == 100)? 0x00: 0x01;
 
        if (bins.max-bins.noise_max >= threshold) {
            return BCD::int_to_bcd((bins.max_index + bins.tick + 1) % number_of_bins + offset);
        } else {
            BCD::bcd_t undefined;
            undefined.val = 0xff;
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
 
        Serial.print(get_time_value(bins).val, HEX);
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
 
namespace DCF77_Encoder {
    using namespace DCF77;
 
    void reset(DCF77::time_data &now) {
        now.second     = 0;
        now.minute.val = 0x00;
        now.hour.val   = 0x00;
 
        now.undefined_minute_output = false;
    }
    void advance_second(DCF77::time_data &now) {
        // in case some value is out of range it will not be advanced
        // this is on purpose
        if (now.second < 59) {
            ++now.second;
        } else {
            now.second = 0;
 
            if (now.minute.val < 0x59) {
                increment(now.minute);
            } else if (now.minute.val == 0x59) {
                now.minute.val = 0x00;
 
                if (now.hour.val < 0x23) {
                    increment(now.hour);
                } else if (now.hour.val == 0x23) {
                    now.hour.val = 0x00;
                }
            }
        }
    }
 
    uint8_t get_current_signal(const DCF77::time_data &now) {
        using namespace Arithmetic_Tools;
 
        if (now.second >= 1 && now.second <= 14) {
            // weather data or other stuff we can not compute
            return undefined;
        }
 
        bool result;
        switch (now.second) {
            case 0:  // start of minute
                return short_tick;
 
            case 20:  // start of time information
                return long_tick;
 
            case 21:
                if (now.undefined_minute_output || now.minute.val > 0x59) { return undefined; }
                result = now.minute.digit.lo & 0x1; break;
            case 22:
                if (now.undefined_minute_output || now.minute.val > 0x59) { return undefined; }
                result = now.minute.digit.lo & 0x2; break;
            case 23:
                if (now.undefined_minute_output || now.minute.val > 0x59) { return undefined; }
                result = now.minute.digit.lo & 0x4; break;
            case 24:
                if (now.undefined_minute_output || now.minute.val > 0x59) { return undefined; }
                result = now.minute.digit.lo & 0x8; break;
 
            case 25:
                if (now.undefined_minute_output || now.minute.val > 0x59) { return undefined; }
                result = now.minute.digit.hi & 0x1; break;
            case 26:
                if (now.undefined_minute_output || now.minute.val > 0x59) { return undefined; }
                result = now.minute.digit.hi & 0x2; break;
            case 27:
                if (now.undefined_minute_output || now.minute.val > 0x59) { return undefined; }
                result = now.minute.digit.hi & 0x4; break;
 
            case 28:
                if (now.undefined_minute_output || now.minute.val > 0x59) { return undefined; }
                result = parity(now.minute.val); break;
 
            case 29:
                if (now.hour.val > 0x23) { return undefined; }
                result = now.hour.digit.lo & 0x1; break;
            case 30:
                if (now.hour.val > 0x23) { return undefined; }
                result = now.hour.digit.lo & 0x2; break;
            case 31:
                if (now.hour.val > 0x23) { return undefined; }
                result = now.hour.digit.lo & 0x4; break;
            case 32:
                if (now.hour.val > 0x23) { return undefined; }
                result = now.hour.digit.lo & 0x8; break;
 
            case 33:
                if (now.hour.val > 0x23) { return undefined; }
                result = now.hour.digit.hi & 0x1; break;
            case 34:
                if (now.hour.val > 0x23) { return undefined; }
                result = now.hour.digit.hi & 0x2; break;
 
            case 35:
                if (now.hour.val > 0x23) { return undefined; }
                result = parity(now.hour.val); break;
            case 59:
                return sync_mark;
 
            default:
                return undefined;
        }
 
        return result? long_tick: short_tick;
    }
 
    void debug(const DCF77::time_data &clock) {
        using namespace Debug;
 
        bcddigits(clock.hour.val);
        Serial.print(':');
        bcddigits(clock.minute.val);
        Serial.print(':');
        if (clock.second < 10) {
            Serial.print('0');
        }
        Serial.println(clock.second, DEC);
   }
 
    void debug(const DCF77::time_data &clock, const uint16_t cycles) {
        DCF77::time_data local_clock = clock;
        DCF77::time_data decoded_clock;
 
        Serial.print(F("M ?????????????? RAZZA S mmmmMMMP hhhhHHP ddddDD www mmmmM yyyyYYYYP S"));
        for (uint16_t second = 0; second < cycles; ++second) {
            switch (local_clock.second) {
                case  0: Serial.println(); break;
                case  1: case 15: case 20: case 21: case 29:
                case 36: case 42: case 45: case 50: case 59: Serial.print(' ');
            }
 
            const uint8_t tick_data = get_current_signal(local_clock);
            Debug::debug_helper(tick_data);
       }
 
        Serial.println();
        Serial.println();
    }
}
namespace DCF77_Hour_Decoder {
    const uint8_t hours_per_day = 24;
 
    typedef struct {
        uint8_t data[hours_per_day];
        uint8_t tick;
 
        uint8_t noise_max;
        uint8_t max;
        uint8_t max_index;
    } hour_bins;
 
    hour_bins bins;
 
 
    void advance_hour() {
        Hamming::advance_tick(bins);
    }
 
    void process_tick(const uint8_t current_second, const uint8_t tick_value) {
        using namespace Hamming;
 
        static BCD::bcd_t hour_data;
 
        switch (current_second) {
            case 29: hour_data.val +=      tick_value; break;
            case 30: hour_data.val +=  0x2*tick_value; break;
            case 31: hour_data.val +=  0x4*tick_value; break;
            case 32: hour_data.val +=  0x8*tick_value; break;
            case 33: hour_data.val += 0x10*tick_value; break;
            case 34: hour_data.val += 0x20*tick_value; break;
            case 35: hour_data.val += 0x80*tick_value;        // Parity !!!
                    hamming_binning<hour_bins, 7, true>(bins, hour_data); break;
 
            case 36: compute_max_index(bins);
                    // fall through on purpose
            default: hour_data.val = 0;
        }
    }
 
    void get_quality(Hamming::lock_quality_t &lock_quality) {
        Hamming::get_quality(bins, lock_quality);
    }
 
    BCD::bcd_t get_hour() {
        return Hamming::get_time_value(bins);
    }
 
    void setup() {
        Hamming::setup(bins);
    }
 
    void debug() {
        Serial.print(F("Hour: "));
        Hamming::debug(bins);
    }
}
 
namespace DCF77_Minute_Decoder {
    const uint8_t minutes_per_hour = 60;
 
    typedef struct {
        uint8_t data[minutes_per_hour];
        uint8_t tick;
 
        uint8_t noise_max;
        uint8_t max;
        uint8_t max_index;
    } minute_bins;
 
    minute_bins bins;
 
    void advance_minute() {
        Hamming::advance_tick(bins);
    }
 
    void process_tick(const uint8_t current_second, const uint8_t tick_value) {
        using namespace Hamming;
 
        static BCD::bcd_t minute_data;
 
        switch (current_second) {
            case 21: minute_data.val +=      tick_value; break;
            case 22: minute_data.val +=  0x2*tick_value; break;
            case 23: minute_data.val +=  0x4*tick_value; break;
            case 24: minute_data.val +=  0x8*tick_value; break;
            case 25: minute_data.val += 0x10*tick_value; break;
            case 26: minute_data.val += 0x20*tick_value; break;
            case 27: minute_data.val += 0x40*tick_value; break;
            case 28: minute_data.val += 0x80*tick_value;        // Parity !!!
                    hamming_binning<minute_bins, 8, true>(bins, minute_data); break;
            case 29: compute_max_index(bins);
            // fall through on purpose
            default: minute_data.val = 0;
        }
    }
 
    void setup() {
        Hamming::setup(bins);
    }
 
    void get_quality(Hamming::lock_quality_t &lock_quality) {
        Hamming::get_quality(bins, lock_quality);
    }
 
    BCD::bcd_t get_minute() {
        return Hamming::get_time_value(bins);
    }
 
    void debug() {
        Serial.print(F("Minute: "));
        Hamming::debug(bins);
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
 
    void set_DCF77_encoder(DCF77::time_data &now) {
        using namespace DCF77_Second_Decoder;
        using namespace DCF77_Minute_Decoder;
        using namespace DCF77_Hour_Decoder;
 
        now.second  = get_second();
        now.minute  = get_minute();
        now.hour    = get_hour();
    }
 
    void flush() {
        // this is called at the end of each second / before the next second begins
        // it is most interesting to propagate this further at the end of a sync marks
        // it is also interesting to propagate this to reference clocks
        DCF77::time_data now;
        DCF77::time_data now_1;
 
        set_DCF77_encoder(now);
        now_1 = now;
 
        DCF77_Encoder::advance_second(now);
 
        decoded_time.second = now.second;
        if (now.second == 0) {
            // the decoder will always decode the data for the NEXT minute
            // thus we have to keep the data of the previous minute
            decoded_time = now_1;
            decoded_time.second = 0;
        }
 
        second_toggle = !second_toggle;
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
        DCF77_Minute_Decoder::get_quality(clock_quality.minute);
        DCF77_Hour_Decoder::get_quality(clock_quality.hour);
    }
 
    void setup() {
        DCF77_Second_Decoder::setup();
        DCF77_Minute_Decoder::setup();
        DCF77_Hour_Decoder::setup();
 
        DCF77_Encoder::reset(decoded_time);
    }
 
    void process_single_tick_data(const uint8_t tick_data) {
        using namespace DCF77;
        using namespace DCF77_Second_Decoder;
        using namespace DCF77_Minute_Decoder;
        using namespace DCF77_Hour_Decoder;
 
        time_data now;
        set_DCF77_encoder(now);
 
        DCF77_Encoder::advance_second(now);
 
        DCF77_Second_Decoder::process_single_tick_data(tick_data);
 
        if (now.second == 0) {
            DCF77_Minute_Decoder::advance_minute();
            if (now.minute.val == 0x00) {
 
                // "while" takes automatically care of timezone change
                while (get_hour().val <= 0x23 && get_hour().val != now.hour.val) { advance_hour(); }
            }
        }
        const uint8_t tick_value = (tick_data == long_tick || tick_data == undefined)? 1: 0;
        DCF77_Minute_Decoder::process_tick(now.second, tick_value);
        DCF77_Hour_Decoder::process_tick(now.second, tick_value);
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
 
 
const uint8_t dcf77_sample_pin = 19; // A5
const uint8_t dcf77_analog_sample_pin = 5;
const uint8_t dcf77_monitor_pin = 18;
 
 
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
 
DCF77::time_data clock;
 
void setup() {
    using namespace DCF77_Encoder;
 
    Serial.begin(115200);
    Serial.println();
 
    pinMode(dcf77_monitor_pin, OUTPUT);
 
    pinMode(dcf77_sample_pin, INPUT);
    digitalWrite(dcf77_sample_pin, HIGH);
 
    for (uint8_t pin=first_output_pin; pin<first_output_pin+8; ++pin) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
    }
 
    DCF77_Demodulator::setup();
    DCF77_Clock_Controller::set_flush_handler(&output_handler);
 
    #ifndef DEBUG
    Serial.flush();
 
    initTimer2();
 
    stopTimer0();
    #endif
}
 
#ifdef DEBUG
int free_Ram () {
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
 
bool noise() {
    const uint8_t r = random(97);
    return r < 0? r&1: 0;
}
#endif
 
void loop() {
    //while (tick != 0) {};
 
#ifdef DEBUG
    using namespace Debug;
    Serial.print(F("free Ram: "));
    Serial.println(free_Ram());
 
    for (uint8_t i=0; i<60; ++i) {
        DCF77_Encoder::advance_second(clock);
 
        const uint8_t signal = DCF77_Encoder::get_current_signal(clock);
        Debug::debug_helper(signal);
 
        for (uint8_t passes=0; passes<100; ++passes) {
            DCF77_Demodulator::detector((signal >= DCF77::undefined? 1: 0));//^noise());
        }
        for (uint8_t passes=0; passes<100; ++passes) {
            DCF77_Demodulator::detector((signal == DCF77::long_tick? 1: 0));//^noise());
        }
        for (uint16_t passes=0; passes<800; ++passes) {
            DCF77_Demodulator::detector(0&&noise());
        }
    }
 
    Serial.println();
    Serial.println();
 
    Serial.println(F("-----------"));
    Serial.print(F("reference: hh:mm:ss  "));
    Serial.print(clock.hour.val, HEX);
    Serial.print(':');
    Serial.print(clock.minute.val, HEX);
    Serial.print(':');
    Serial.println(clock.second, DEC);
#endif
 
    DCF77::time_data now;
 
    DCF77_Clock_Controller::get_current_time(now);
    if (now.second <= 60) {
        Serial.println();
        Serial.print(F("Decoded time: "));
 
        DCF77_Encoder::debug(now);
    }
 
    DCF77_Clock_Controller::clock_quality_t clock_quality;
    DCF77_Clock_Controller::get_quality(clock_quality);
 
    Serial.print(F("Quality (p,s,m,h): "));
    Serial.print('(');
    Serial.print(clock_quality.phase.lock_max, DEC);
    Serial.print('-');
    Serial.print(clock_quality.phase.noise_max, DEC);
    Serial.print(')');
 
    Serial.print('(');
    Serial.print(clock_quality.second.lock_max, DEC);
    Serial.print('-');
    Serial.print(clock_quality.second.noise_max, DEC);
    Serial.print(')');
 
    Serial.print('(');
    Serial.print(clock_quality.minute.lock_max, DEC);
    Serial.print('-');
    Serial.print(clock_quality.minute.noise_max, DEC);
    Serial.print(')');
 
    Serial.print('(');
    Serial.print(clock_quality.hour.lock_max, DEC);
    Serial.print('-');
    Serial.print(clock_quality.hour.noise_max, DEC);
    Serial.print(')');
 
    /**/
 
    //DCF77_Hour_Decoder::debug();
    //DCF77_Minute_Decoder::debug();
    //DCF77_Second_Decoder::debug();
}
