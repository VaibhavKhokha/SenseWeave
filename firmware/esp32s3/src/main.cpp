#include <Arduino.h>
#include <Wire.h>
#include "board_pins.h"

namespace {
constexpr uint32_t kSerialBaudRate = 115200;
constexpr uint32_t kScanIntervalMs = 3000;
constexpr uint8_t kMpu6050DefaultAddr = 0x68;
constexpr uint8_t kMpu6050RegWhoAmI = 0x75;

uint32_t g_last_scan_ms = 0;

// Reads a single 8-bit register from an I2C device. Returns true on success.
bool readI2cRegister(uint8_t dev_addr, uint8_t reg_addr, uint8_t* out_val) {
    Wire.beginTransmission(dev_addr);
    Wire.write(reg_addr);
    // Send repeated start (false) so we keep control of the bus for the read
    if (Wire.endTransmission(false) != 0) {
        return false;
    }
    if (Wire.requestFrom(static_cast<int>(dev_addr), 1) != 1) {
        return false;
    }
    *out_val = Wire.read();
    return true;
}

void runI2cDiagnostics() {
    Serial.println("----------------------------------------------------");
    Serial.printf("[SenseWeave I2C Scan] Bus: SDA=GPIO%d, SCL=GPIO%d @ %u Hz\n",
                  senseweave::pins::kI2cSdaPin,
                  senseweave::pins::kI2cSclPin,
                  senseweave::pins::kI2cClockHz);

    uint8_t found_count = 0;
    for (uint8_t addr = 0x01; addr < 0x7F; ++addr) {
        Wire.beginTransmission(addr);
        const uint8_t error = Wire.endTransmission();
        if (error == 0) {
            Serial.printf("  -> Found I2C device at address 0x%02X\n", addr);
            ++found_count;
        }
    }

    if (found_count == 0) {
        Serial.println("  [ERROR] No I2C devices detected! Check VCC, GND, SDA(GPIO8), SCL(GPIO9).");
        return;
    }

    uint8_t who_am_i = 0x00;
    if (readI2cRegister(kMpu6050DefaultAddr, kMpu6050RegWhoAmI, &who_am_i)) {
        Serial.printf("  [OK] MPU6050 (0x68) WHO_AM_I (Reg 0x75) = 0x%02X\n", who_am_i);
    } else {
        Serial.println("  [WARN] Could not read WHO_AM_I register from 0x68.");
    }
}
}  // namespace

void setup() {
    Serial.begin(kSerialBaudRate);
    delay(1000);

    // Initialize I2C bus on explicit SenseWeave pins
    Wire.begin(senseweave::pins::kI2cSdaPin,
               senseweave::pins::kI2cSclPin,
               senseweave::pins::kI2cClockHz);

    runI2cDiagnostics();
}

void loop() {
    const uint32_t now_ms = millis();
    if (now_ms - g_last_scan_ms >= kScanIntervalMs) {
        g_last_scan_ms = now_ms;
        runI2cDiagnostics();
    }
}