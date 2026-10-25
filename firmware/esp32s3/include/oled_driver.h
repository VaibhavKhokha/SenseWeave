#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

namespace senseweave
{
    class OledDriver
    {
        public:
            OledDriver() : display_(128, 64, &Wire, -1) {}

            bool begin()
            {
                if(!display_.begin(SSD1306_SWITCHCAPVCC, 0x3C))
                {
                    return false;
                }

                display_.clearDisplay();
                display_.setTextColor(SSD1306_WHITE);
                display_.setTextSize(1);
                display_.setCursor(0, 0);
                display_.println("SenseWeave");
                display_.println("Booting...");
                display_.display();
                return true;

            }

            void printResult(const char* state_name)
            {
                display_.clearDisplay();
                display_.setTextSize(1);
                display_.setCursor(0, 0);
                display_.println(state_name);

                display_.display();

            }

        private:
            Adafruit_SSD1306 display_;

    };

}
