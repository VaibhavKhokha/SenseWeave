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
    constexpr uint32_t kPrintIntervalMs = 20;

    senseweave::Mpu6050Driver g_imu;
    senseweave::LdrDriver g_ldr;
    senseweave::Hcsr04Driver g_usonic;
    senseweave::VibrationDriver g_vib;

    uint32_t g_last_sample_ms = 0;
    bool g_is_recording = false;
    int g_current_label = 0;

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
    if(Serial.available() > 0)
    {
        char cmd = Serial.read();
        if(cmd >= '0' && cmd <= '4')
        {
            g_current_label = cmd - '0';
            g_is_recording = true;
            Serial.println("--- RECORDING STARTED ---");
        }
        else if(cmd == 's' || cmd == 'S')
        {
            g_is_recording = false;
            Serial.println("--- RECORDING STOPPED ---");
        }

    }

    const uint32_t now_ms = millis();

    if(now_ms - g_last_sample_ms >= kPrintIntervalMs)
    {
        g_last_sample_ms = now_ms;

        senseweave::ImuSample sample{};

        uint32_t ldr_val = 0;
        uint32_t ldr_latency = 0;

        static float dist_cm = 0.0f;
        static uint32_t dist_latency = 0;
        static uint32_t last_usonic_read_ms = 0;

        uint8_t vib_val = 0;
        uint32_t vib_latency = 0;

        if(!g_imu.readSample(&sample)) Serial.println("Error: I2C read failed...");
        if(!g_ldr.readSample(&ldr_val, &ldr_latency)) Serial.println("LDR Read Failed...");
        if(!g_vib.readSample(&vib_val, &vib_latency)) Serial.println("VIB Read Failed...");

        if(now_ms - last_usonic_read_ms >= 200)
        {
            last_usonic_read_ms = now_ms;
            if(!g_usonic.readSample(&dist_cm, &dist_latency))
            {
                Serial.println("Ultrasonic Read Failed...");
            }

        }

        if(g_is_recording)
        {
            Serial.printf("%lu,%d,%.3f,%.3f,%.3f,%.2f,%.2f,%.2f,%u,%.1f,%d\n",
                          now_ms, g_current_label,
                          sample.ax_g, sample.ay_g, sample.az_g,
                          sample.gx_dps, sample.gy_dps, sample.gz_dps,
                          ldr_val, dist_cm, vib_val);
        }

    }

}
