#pragma once
#include <Arduino.h>
#include <esp_timer.h>

namespace senseweave
{
    class LdrDriver
    {
        public:
            void begin(int pin)
            {
                pin_ = pin;
                analogReadResolution(12);
            }

            bool readSample(uint32_t* out_val, uint32_t* out_latency_us)
            {
                const int64_t t_start = esp_timer_get_time();
                *out_val = analogRead(pin_);

                const int64_t t_end = esp_timer_get_time();
                *out_latency_us = static_cast<uint32_t>(t_end - t_start);
                return true;

            }

        private:
            int pin_ = -1;

    };

}