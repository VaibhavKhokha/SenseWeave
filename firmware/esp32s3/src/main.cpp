#include <Arduino.h>
#include <Wire.h>
#include "board_pins.h"
#include "mpu6050_driver.h"

namespace
{
    constexpr uint32_t kSerialBaudRate = 115200;
    constexpr uint32_t kPrintIntervalMs = 200;

    senseweave::Mpu6050Driver g_imu;
    uint32_t g_last_sample_ms = 0;    
}

void setup()
{
    
}
