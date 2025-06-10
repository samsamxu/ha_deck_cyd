#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "LGFX.h"
#include "lvgl.h"
#include <Wire.h>

// 删除XPT2046相关定义
// 添加CST816 I2C地址
#define CST816_I2C_ADDR 0x15

namespace esphome {
namespace hd_device {

class CST816Touch {
public:
    void begin(uint8_t sda_pin, uint8_t scl_pin, uint8_t rst_pin = -1) {
        this->sda_pin_ = sda_pin;
        this->scl_pin_ = scl_pin;
        this->rst_pin_ = rst_pin;
        
        if (rst_pin != -1) {
            pinMode(rst_pin, OUTPUT);
            digitalWrite(rst_pin, LOW);
            delay(10);
            digitalWrite(rst_pin, HIGH);
            delay(50);
        }
        
        Wire.begin(sda_pin, scl_pin);
        delay(50); // 确保设备初始化完成
    }

    bool touched() {
        uint8_t data[1];
        if (!readRegister(0x02, data, 1)) return false;
        return (data[0] & 0x80) != 0;
    }

    bool getTouch(int16_t *x, int16_t *y) {
        uint8_t data[6];
        if (!readRegister(0x03, data, 6)) return false;
        
        uint8_t status = data[0];
        if (!(status & 0x80)) return false; // 检查触摸有效位
        
        *x = ((data[1] & 0x0F) << 8) | data[2];
        *y = ((data[3] & 0x0F) << 8) | data[4];
        
        // 根据实际屏幕方向调整坐标
        applyRotation(x, y);
        return true;
    }

    void setRotation(uint8_t rotation) { rotation_ = rotation; }

private:
    uint8_t sda_pin_, scl_pin_, rst_pin_;
    uint8_t rotation_ = 0;

    void applyRotation(int16_t *x, int16_t *y) {
        int16_t temp;
        switch (rotation_) {
            case 0: // 0°
                break;
            case 1: // 90°
                temp = *x;
                *x = *y;
                *y = 240 - temp; // 根据实际分辨率调整
                break;
            case 2: // 180°
                *x = 320 - *x;   // 根据实际分辨率调整
                *y = 240 - *y;
                break;
            case 3: // 270°
                temp = *x;
                *x = 320 - *y;
                *y = temp;
                break;
        }
    }

    bool readRegister(uint8_t reg, uint8_t *data, uint8_t len) {
        Wire.beginTransmission(CST816_I2C_ADDR);
        Wire.write(reg);
        if (Wire.endTransmission(false) != 0) return false;
        
        Wire.requestFrom(CST816_I2C_ADDR, len);
        for (int i = 0; i < len; i++) {
            data[i] = Wire.read();
        }
        return true;
    }
};

class HaDeckDevice : public Component
{
public:
    void setup() override;
    void loop() override;
    float get_setup_priority() const override;
    uint8_t get_brightness();
    void set_brightness(uint8_t value);
    
    // 添加触摸对象
    CST816Touch touch;

private:
    unsigned long time_ = 0;
    uint8_t brightness_ = 0;
};

}  // namespace hd_device
}  // namespace esphome
