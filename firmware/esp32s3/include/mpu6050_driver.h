#pragma once
#include <Wire.h>
#include <Arduino.h>
#include <esp_timer.h>
#include <stdint.h>

namespace senseweave {

struct ImuSample
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

        bool readSample(ImuSample* out_sample)
        {
            if(out_sample == nullptr) return false;
            
            const int64_t t_start_us = esp_timer_get_time();

            Wire.beginTransmission(dev_addr_);
            Wire.write(kRegAccelXoutH);

            if(Wire.endTransmission(false) != 0)
            {
                return false;
            }

            constexpr uint8_t kBurstBytes = 14;

            if(Wire.requestFrom(static_cast<int>(dev_addr_), static_cast<int>(kBurstBytes)) != kBurstBytes)
            {
                return false;
            }

            uint8_t buf[kBurstBytes];
            for(uint8_t i =0; i<kBurstBytes; ++i)
            {
                buf[i] = Wire.read();
            }

            const int16_t raw_ax = unpackInt16(buf[0], buf[1]);
            const int16_t raw_ay = unpackInt16(buf[2], buf[3]);
            const int16_t raw_az = unpackInt16(buf[4], buf[5]);

            const int16_t raw_gx = unpackInt16(buf[8], buf[9]);
            const int16_t raw_gy = unpackInt16(buf[10], buf[11]);
            const int16_t raw_gz = unpackInt16(buf[12], buf[13]);
            
            out_sample->ax_g = static_cast<float>(raw_ax) / kAccelScaleLsbPerG;
            out_sample->ay_g = static_cast<float>(raw_ay) / kAccelScaleLsbPerG;
            out_sample->az_g = static_cast<float>(raw_az) / kAccelScaleLsbPerG;

            out_sample->gx_dps = static_cast<float>(raw_gx) / kGyroScaleLsbPerDps;
            out_sample->gy_dps = static_cast<float>(raw_gy) / kGyroScaleLsbPerDps;
            out_sample->gz_dps = static_cast<float>(raw_gz) / kGyroScaleLsbPerDps;

            const int64_t t_end_us = esp_timer_get_time();

            out_sample->acquire_latency_us = static_cast<uint32_t>(t_end_us - t_start_us);
            
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

        static int16_t unpackInt16(uint8_t hi, uint8_t lo)
        {
            return static_cast<int16_t>((static_cast<uint16_t>(hi) << 8) | lo);
        }


};

}