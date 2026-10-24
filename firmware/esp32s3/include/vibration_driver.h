#pragma once
#include <Arduino.h>
#include <esp_timer.h>

namespace senseweave
{
    class VibrationDriver
    {
        public:
            void begin(int pin)
            {
                pin_ = pin;

                pinMode(pin_, INPUT_PULLUP);

            }

            bool readSample(uint8_t* out_val, uint32_t* out_latency_us)
            {
                const int64_t t_start = esp_timer_get_time();

                *out_val = digitalRead(pin_);

                const int64_t t_end = esp_timer_get_time();

                *out_latency_us = static_cast<uint32_t>(t_end - t_start);

                return true;
            }


        private:
            int pin_ = -1;

    };

}