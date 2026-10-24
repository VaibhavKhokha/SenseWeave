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
    Serial.begin(kSerialBaudRate);
    delay(1000);

    Wire.begin(senseweave::pins::kI2cSdaPin,
                senseweave::pins::kI2cSclPin,
                senseweave::pins::kI2cClockHz);

    Serial.println("======================================");
    if(g_imu.begin())
    {
        Serial.printf("IMU Working -- > 0x%02X \n", g_imu.whoAmI());
    }
    else
    {
        Serial.println("Error: Failed to Initialize MPU6050");
    }

    Serial.println("=========================================");

}


void loop()
{
    const uint32_t now_ms = millis();

    if(now_ms - g_last_sample_ms >= kPrintIntervalMs)
    {
        g_last_sample_ms = now_ms;

        senseweave::ImuSample sample{};

        if(g_imu.readSample(&sample))
        {
            Serial.printf("[IMU] Accel(g): X=%+6.3f Y=%+6.3f Z=%+6.3f | "
                          "Gyro(dps): X=%+7.2f Y=%+7.2f Z=%+7.2f | "
                          "T_acq=%u us\n",
                          sample.ax_g, sample.ay_g, sample.az_g,
                          sample.gx_dps, sample.gy_dps, sample.gz_dps,
                          sample.acquire_latency_us);
        }
        else
        {
            Serial.println("Error: I2C read failed...");
        }

    }

}
