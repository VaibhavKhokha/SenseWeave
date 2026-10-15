#include <Arduino.h>
#include <esp_timer.h>

namespace {
constexpr uint32_t kSerialBaudRate = 115200;
constexpr uint32_t kHeartbeatIntervalMs = 2000;
uint32_t g_last_report_ms = 0;
}  // namespace

void printHardwareReport() {
    const int64_t uptime_us = esp_timer_get_time();

    Serial.println("====================================================");
    Serial.println("[SenseWeave] ESP32-S3-N16R8 Hardware Diagnostic");
    Serial.printf("  Chip Model      : %s (Rev %d, %d Cores)\n",
                  ESP.getChipModel(), ESP.getChipRevision(), ESP.getChipCores());
    Serial.printf("  CPU Frequency   : %u MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("  Flash Size      : %u Bytes (%.2f MB)\n",
                  ESP.getFlashChipSize(),
                  ESP.getFlashChipSize() / (1024.0f * 1024.0f));
    Serial.printf("  Internal SRAM   : %u Bytes Free / %u Bytes Total\n",
                  ESP.getFreeHeap(), ESP.getHeapSize());
    Serial.printf("  Octal PSRAM     : %u Bytes Free / %u Bytes Total\n",
                  ESP.getFreePsram(), ESP.getPsramSize());
    Serial.printf("  High-Res Timer  : %lld us since boot\n", uptime_us);
    Serial.println("====================================================");
}

void setup() {
    Serial.begin(kSerialBaudRate);
    delay(1000);  // Allow serial monitor time to attach after reset
    printHardwareReport();
}

void loop() {
    const uint32_t now_ms = millis();
    if (now_ms - g_last_report_ms >= kHeartbeatIntervalMs) {
        g_last_report_ms = now_ms;
        printHardwareReport();
    }
}