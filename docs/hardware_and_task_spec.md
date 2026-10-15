# SenseWeave Hardware Reference & Task Specification

## 1. Target MCU: ESP32-S3-N16R8 (3.3 V Logic)
- Forbidden Octal PSRAM Pins: GPIO35, GPIO36, GPIO37
- Reserved Strapping Pins: GPIO0, GPIO3, GPIO45, GPIO46
- Reserved USB/UART Pins: GPIO19, GPIO20, GPIO43, GPIO44

## 2. Core Sensor Pin Assignments (3.3 V Rail)
- MPU6050 (I2C 0x68): VCC->3V3, GND->GND, SDA->GPIO8, SCL->GPIO9
- Bare LDR + 10kOhm Divider: 3V3 -> LDR -> GPIO4 (ADC1_3) -> 10kOhm -> GND
- HC-SR04 (RCWL-9300D at 3.3V): VCC->3V3, GND->GND, Trig->GPIO5, Echo->GPIO6

## 3. Demonstration Task (4 Classes, Session-Split Dataset)
- Labels: IDLE_AMBIENT, LOW_LIGHT_IDLE, PROXIMITY_HOVER, ACTIVE_WORK_VIBRATION
- Default Constraints: Accuracy >= 85.0%, Tensor Arena <= 16384 B, Latency Profile 1 <= 15000 us, Profile 2 <= 2500 us
