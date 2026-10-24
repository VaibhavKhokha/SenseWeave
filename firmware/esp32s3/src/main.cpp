#include <Arduino.h>
#include <Wire.h>
#include "board_pins.h"
#include "mpu6050_driver.h"
#include "ldr_driver.h"
#include "hcsr04_driver.h"
#include "vibration_driver.h"

namespace
{
    constexpr uint32_t kSerialBaudRate = 115200;
    constexpr uint32_t kPrintIntervalMs = 200;

    senseweave::Mpu6050Driver g_imu;
    senseweave::LdrDriver g_ldr;
    senseweave::Hcsr04Driver g_usonic;
    senseweave::VibrationDriver g_vib;

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


    g_ldr.begin(senseweave::pins::kLdrAdcPin);
    Serial.println("LDR Initialized...");
    Serial.println("=========================================");

    g_usonic.begin(senseweave::pins::kUltrasonicTrigPin, senseweave::pins::kUltrasonicEchoPin);
    Serial.println("Ultrasonic Sensor Initialized...");
    Serial.println("==========================================");

    g_vib.begin(senseweave::pins::kVibrationSwitchPin);
    Serial.println("Vibration Sensor Initialized...");
    Serial.println("==========================================");

}


void loop()
{
    const uint32_t now_ms = millis();

    if(now_ms - g_last_sample_ms >= kPrintIntervalMs)
    {
        g_last_sample_ms = now_ms;

        senseweave::ImuSample sample{};
        uint32_t ldr_val = 0;
        uint32_t ldr_latency = 0;
        float dist_cm = 0.0f;
        uint32_t dist_latency = 0;
        uint8_t vib_val = 0;
        uint32_t vib_latency = 0;

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

        if(g_ldr.readSample(&ldr_val, &ldr_latency))
        {
            Serial.printf("[LDR] val=%4u | T_acq=%3u us\n", ldr_val, ldr_latency);
        }
        else
        {
            Serial.println("LDR Read Failed...");
        }

        if(g_usonic.readSample(&dist_cm, &dist_latency))
        {
            Serial.printf("[DIST] %6.1f cm | T_acq = %5u us \n", dist_cm, dist_latency);
        }
        else
        {
            Serial.println("ULtrasonic Read Failed...");
        }

        if(g_vib.readSample(&vib_val, &vib_latency))
        {
            Serial.printf("[VIB] %d T = %u us \n", vib_val, vib_latency);
        }

    }

}
