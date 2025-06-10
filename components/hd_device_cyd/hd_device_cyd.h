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

#include "CST816S.h"

class CST816Touch : public CST816S {
public:
    CST816Touch() : CST816S(21, 22, -1, -1) {} // 使用默认引脚，可根据实际接线修改
    
    void begin(uint8_t sda_pin, uint8_t scl_pin, uint8_t rst_pin = -1) {
        // 初始化I2C引脚
        Wire.begin(sda_pin, scl_pin);
        // 调用基类begin方法
        CST816S::begin();
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
