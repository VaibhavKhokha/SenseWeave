#pragma once
#include <stdint.h>

namespace senseweave {
namespace pins {

// I2C Bus 0 Pins (3.3V Logic — Connected to MPU6050)
constexpr int kI2cSdaPin = 8;   // GPIO8
constexpr int kI2cSclPin = 9;   // GPIO9
constexpr uint32_t kI2cClockHz = 400000;  // 400 kHz Fast Mode I2C

// Analog Sensor Pins (3.3V Logic — Bare LDR + 10kOhm Voltage Divider)
constexpr int kLdrAdcPin = 1;   // GPIO1 (ADC1_CH0)

// Ultrasonic Sensor Pins (3.3V Supply & Logic — HC-SR04 with RCWL-9300D)
constexpr int kUltrasonicTrigPin = 17;  // GPIO17
constexpr int kUltrasonicEchoPin = 18;  // GPIO18

// Vibration Sensor
constexpr int kVibrationSwitchPin = 7; // GPIO7

}  // namespace pins
}  // namespace senseweave