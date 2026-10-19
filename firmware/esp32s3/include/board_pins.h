#pragma once
#include <stdint.h>

namespace senseweave {
namespace pins {

// I2C Bus 0 Pins (3.3V Logic — Connected to MPU6050)
constexpr int kI2cSdaPin = 8;   // GPIO8
constexpr int kI2cSclPin = 9;   // GPIO9
constexpr uint32_t kI2cClockHz = 400000;  // 400 kHz Fast Mode I2C

// Analog Sensor Pins (3.3V Logic — Bare LDR + 10kOhm Voltage Divider)
constexpr int kLdrAdcPin = 4;   // GPIO4 (ADC1_CH3)

// Ultrasonic Sensor Pins (3.3V Supply & Logic — HC-SR04 with RCWL-9300D)
constexpr int kUltrasonicTrigPin = 5;  // GPIO5
constexpr int kUltrasonicEchoPin = 6;  // GPIO6

}  // namespace pins
}  // namespace senseweave