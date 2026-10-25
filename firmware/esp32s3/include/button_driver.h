#pragma once
#include <Arduino.h>

namespace senseweave
{
    class ButtonDriver
    {
        public:
            void begin(int pin)
            {
                pin_ = pin;
                pinMode(pin_, INPUT_PULLUP);
                last_debounce_time_ = 0;
                button_state_ = HIGH;
                last_button_state_ = HIGH;

            }

            bool isPressed()
            {
                bool pressed_event = false;
                int reading = digitalRead(pin_);

                if(reading != last_button_state_)
                {
                    last_debounce_time_ = millis();
                }

                if((millis() - last_debounce_time_) > debounce_delay_)
                {
                    if(reading != button_state_)
                    {
                        button_state_ = reading;

                        if(button_state_ == LOW)
                        {
                            pressed_event = true;
                        }

                    }
                }

                last_button_state_ = reading;
                return pressed_event;

            }

        private:
            int pin_ = -1;
            int button_state_;
            int last_button_state_;
            uint32_t last_debounce_time_;
            const uint32_t debounce_delay_ = 50;

    };

}