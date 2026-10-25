#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include "board_pins.h"
#include "mpu6050_driver.h"
#include "ldr_driver.h"
#include "hcsr04_driver.h"
#include "vibration_driver.h"
#include "button_driver.h"
#include "oled_driver.h"
#include "model.h"

namespace
{
    constexpr uint32_t kSerialBaudRate = 115200;
    constexpr uint32_t kPrintIntervalMs = 20;
    constexpr int kButtonPin = 10;

    senseweave::Mpu6050Driver g_imu;
    senseweave::LdrDriver g_ldr;
    senseweave::Hcsr04Driver g_usonic;
    senseweave::VibrationDriver g_vib;
    senseweave::ButtonDriver g_button;
    senseweave::OledDriver g_oled;

    Eloquent::ML::Port::SenseWeaveModel ml_model;

    uint32_t g_last_sample_ms = 0;
    bool g_is_monitoring = false;
    int g_sample_count = 0;

    float ax_buf[50], ay_buf[50], az_buf[50];
    float gx_buf[50], gy_buf[50], gz_buf[50];
    float ldr_buf[50], dist_buf[50], vib_buf[50];

    const char* serial_class_names[] = {
        "0: [NORMAL] Resting Safely",
        "1: [TRANSIT] Moving in Truck",
        "2: [TILT] Tipped Over!",
        "3: [OPENED] Box Breached (Light)",
        "4: [IMPACT] Dropped/Rough Handling!"
    };

    const char* oled_class_names[] = {
        "NORMAL",
        "TRANSIT",
        "TILTED!",
        "OPENED!",
        "IMPACT!"
    };

    void compute_stats(float* data, int n, float& mean, float& std_dev, float& min_val, float& max_val)
    {
        float sum = 0.0f;
        min_val = data[0];
        max_val = data[0];
        for(int i = 0; i < n; i++)
        {
            sum += data[i];
            if(data[i] < min_val) min_val = data[i];
            if(data[i] > max_val) max_val = data[i];
        }
        mean = sum / n;
        
        float variance_sum = 0.0f;
        for(int i = 0; i < n; i++)
        {
            variance_sum += (data[i] - mean) * (data[i] - mean);
        }
        std_dev = sqrt(variance_sum / (n - 1)); 
    }
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

    g_button.begin(kButtonPin);
    Serial.println("Button Initialized...");
    Serial.println("==========================================");

    if(g_oled.begin())
    {
        Serial.println("OLED Initialized...");
    }
    else
    {
        Serial.println("OLED Failed...");
    }
    Serial.println("==========================================");
    
    g_oled.printResult("PRESS BTN");
}

void loop()
{
    if(g_button.isPressed())
    {
        g_is_monitoring = !g_is_monitoring;
        if(g_is_monitoring)
        {
            Serial.println("--- MONITORING STARTED ---");
            g_oled.printResult("ARMED...");
            g_sample_count = 0;
            g_last_sample_ms = millis();
        }
        else
        {
            Serial.println("--- MONITORING STOPPED ---");
            g_oled.printResult("PAUSED");
        }
    }

    if(!g_is_monitoring)
    {
        return; 
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
            }
        }

        ax_buf[g_sample_count] = sample.ax_g;
        ay_buf[g_sample_count] = sample.ay_g;
        az_buf[g_sample_count] = sample.az_g;
        gx_buf[g_sample_count] = sample.gx_dps;
        gy_buf[g_sample_count] = sample.gy_dps;
        gz_buf[g_sample_count] = sample.gz_dps;
        ldr_buf[g_sample_count] = (float)ldr_val;
        dist_buf[g_sample_count] = dist_cm;
        vib_buf[g_sample_count] = (float)vib_val;

        g_sample_count++;

        if(g_sample_count == 50)
        {
            float features[27];

            compute_stats(ax_buf, 50, features[0], features[1], features[2], features[3]);
            compute_stats(ay_buf, 50, features[4], features[5], features[6], features[7]);
            compute_stats(az_buf, 50, features[8], features[9], features[10], features[11]);
            compute_stats(gx_buf, 50, features[12], features[13], features[14], features[15]);
            compute_stats(gy_buf, 50, features[16], features[17], features[18], features[19]);
            compute_stats(gz_buf, 50, features[20], features[21], features[22], features[23]);

            float ldr_sum = 0; 
            for(int i=0; i<50; i++) ldr_sum += ldr_buf[i];
            features[24] = ldr_sum / 50.0f;

            float dist_sum = 0; 
            for(int i=0; i<50; i++) dist_sum += dist_buf[i];
            features[25] = dist_sum / 50.0f;

            float vib_max_val = 0; 
            for(int i=0; i<50; i++) 
            {
                if(vib_buf[i] > vib_max_val) vib_max_val = vib_buf[i];
            }
            features[26] = vib_max_val;

            int32_t t_start = micros();
            int prediction = ml_model.predict(features);
            int32_t t_latency = micros() - t_start;

            Serial.printf("--> Prediction: %s | Time: %d us\n", serial_class_names[prediction], t_latency);
            g_oled.printResult(oled_class_names[prediction]);

            g_sample_count = 0;
        }
    }
}