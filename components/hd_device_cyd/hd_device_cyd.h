#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "LGFX.h"
#include "lvgl.h"
#include <Wire.h>

// 添加CST816 I2C地址
#define CST816_I2C_ADDR 0x15

namespace esphome {
namespace hd_device {

#include <Wire.h>

#define CST816_ADDR 0x15
#define TOUCH_REG_GESTURE 0x01
#define TOUCH_REG_FINGERS 0x02
#define TOUCH_REG_XPOS_H 0x03
#define TOUCH_REG_XPOS_L 0x04
#define TOUCH_REG_YPOS_H 0x05
#define TOUCH_REG_YPOS_L 0x06

class CST816Touch {
public:
    CST816Touch() : sda_pin(21), scl_pin(22), rst_pin(-1), irq_pin(-1), last_touch(0) {}
    
    void begin(uint8_t sda, uint8_t scl, uint8_t rst = -1) {
        sda_pin = sda;
        scl_pin = scl;
        rst_pin = rst;
        Wire.begin(sda_pin, scl_pin);
        
        if (rst_pin != -1) {
            pinMode(rst_pin, OUTPUT);
            digitalWrite(rst_pin, LOW);
            delay(10);
            digitalWrite(rst_pin, HIGH);
            delay(50);
        }
        
        // 初始化触摸屏寄存器
        writeRegister(0xFE, 0xFF); // 复位
        delay(120);
    }
    
    bool available() {
        uint8_t gesture = readRegister(TOUCH_REG_GESTURE);
        uint8_t fingers = readRegister(TOUCH_REG_FINGERS);
        
        if (fingers > 0) {
            uint8_t xh = readRegister(TOUCH_REG_XPOS_H);
            uint8_t xl = readRegister(TOUCH_REG_XPOS_L);
            uint8_t yh = readRegister(TOUCH_REG_YPOS_H);
            uint8_t yl = readRegister(TOUCH_REG_YPOS_L);
            
            data.x = ((xh & 0x0F) << 8) | xl;
            data.y = ((yh & 0x0F) << 8) | yl;
            data.event = (xh >> 6) & 0x03;
            data.gestureID = gesture;
            data.points = fingers;
            
            last_touch = millis();
            return true;
        }
        return false;
    }
    
    bool touched() {
        return available();
    }
    
    bool getTouch(int16_t *x, int16_t *y) {
        if (available()) {
            *x = data.x;
            *y = data.y;
            return true;
        }
        return false;
    }

    struct TouchData {
        uint8_t gestureID;
        uint8_t points;
        uint8_t event; // 0:按下 1:抬起 2:接触
        int16_t x;
        int16_t y;
    } data;

private:
    uint8_t readRegister(uint8_t reg) {
        Wire.beginTransmission(CST816_ADDR);
        Wire.write(reg);
        Wire.endTransmission();
        Wire.requestFrom(CST816_ADDR, 1);
        return Wire.read();
    }
    
    void writeRegister(uint8_t reg, uint8_t val) {
        Wire.beginTransmission(CST816_ADDR);
        Wire.write(reg);
        Wire.write(val);
        Wire.endTransmission();
    }

    uint8_t sda_pin;
    uint8_t scl_pin;
    uint8_t rst_pin;
    uint8_t irq_pin;
    unsigned long last_touch;
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
