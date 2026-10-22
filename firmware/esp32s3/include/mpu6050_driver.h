#pragma once
#include <Wire.h>
#include <Arduino.h>
#include <esp_timer.h>
#include <stdint.h>

namespace senseweave {

struct ImpuSample
{
    float ax_g;
    float ay_g;
    float az_g;
    float gx_dps;
    float gy_dps;
    float gz_dps;
    uint32_t acquire_latency_us;
};

class Mpu6050Drive
{
    public:
        static constexpr uint8_t kDefaultAddr = 0x68;
        static constexpr uint8_t kRegGyroConfig = 0x1B;
        static constexpr uint8_t kRegAccelConfig = 0x1C;
        static constexpr uint8_t kRegAccelXoutH = 0x3B;
        static constexpr uint8_t kRegPwrMgmt1 = 0x6B;
        static constexpr uint8_t kRegWhoAmI = 0x75;

        static constexpr float kAccelScaleLsbPerG = 16384.0f;
        static constexpr float kGyroScaleLsbPerDps = 131.0f;

        uint8_t whoAmI() const { return who_am_i_;}

        bool begin(uint8_t addr = kDefaultAddr)
        {
            dev_addr_ = addr;

            if(!readRegister(kRegWhoAmI, &who_am_i_))
            {
                return false;
            }

            if(who_am_i_ != 0x68 && who_am_i_ != 0x70 && who_am_i_ != 0x72 && who_am_i_ != 0x98)
            {
                return false;
            }

            if(!writeRegister(kRegPwrMgmt1, 0x01)) return false;
            delay(50);

            if(!writeRegister(kRegAccelConfig, 0x00)) return false;
            if(!writeRegister(kRegGyroConfig, 0x00)) return false;

            return true;
            
        }


    private:
        uint8_t dev_addr_ = kDefaultAddr;
        uint8_t who_am_i_ = 0x00;

        bool writeRegister(uint8_t reg, uint8_t val)
        {
            Wire.beginTransmission(dev_addr_);
            Wire.write(reg);
            Wire.write(val);
            return (Wire.endTransmission() == 0);
        }

        bool readRegister(uint8_t reg, uint8_t* out_val)
        {
            Wire.beginTransmission(dev_addr_);
            Wire.write(reg);

            if(Wire.endTransmission(false) != 0) return false;

            if(Wire.requestFrom(static_cast<int>(dev_addr_), 1) != 1) return false;
            *out_val = Wire.read();
            return true;
        }


};

}