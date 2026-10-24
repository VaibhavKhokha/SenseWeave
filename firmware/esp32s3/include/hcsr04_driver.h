#pragma once
#include <Arduino.h>
#include <esp_timer.h>

namespace senseweave
{
    class Hcsr04Driver
    {
        public:
            static constexpr uint32_t kTimeoutUS = 25000;

            void begin(int trig_pin, int echo_pin)
            {
                trig_pin_ = trig_pin;
                echo_pin_ = echo_pin;

                pinMode(trig_pin_, OUTPUT);
                pinMode(echo_pin_, INPUT);

                digitalWrite(trig_pin_, LOW);

            }

            bool readSample(float* out_cm, uint32_t* out_latency_us)
            {
                const int64_t t_start = esp_timer_get_time();

                digitalWrite(trig_pin_, HIGH);
                delayMicroseconds(10);
                digitalWrite(trig_pin_, LOW);

                const unsigned long duration_us = pulseIn(echo_pin_, HIGH, kTimeoutUS);

                if(duration_us == 0)
                {
                    *out_cm = -1.0f;
                }
                else
                {
                    *out_cm = (duration_us * 0.0343f) / 2.0f;
                }

                const int64_t t_end = esp_timer_get_time();
                *out_latency_us = static_cast<uint32_t>(t_end - t_start);

                return (duration_us > 0);

            }

        private:
            int trig_pin_ = -1;
            int echo_pin_ = -1;

    };

}