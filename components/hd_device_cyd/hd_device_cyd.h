#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "LGFX.h"
#include "lvgl.h"
#include <Wire.h>  // 添加I2C支持
#include <CST816S.h> // 替换为CST816触摸库

// 删除XPT2046引脚定义
// 添加CST816引脚定义
#define TOUCH_SDA 21
#define TOUCH_SCL 22
#define TOUCH_RST 33
#define TOUCH_INT -1

namespace esphome {
namespace hd_device {

class HaDeckDevice : public Component
{
public:
    void setup() override;
    void loop() override;
    float get_setup_priority() const override;
    uint8_t get_brightness();
    void set_brightness(uint8_t value);
private:
    unsigned long time_ = 0;
    uint8_t brightness_ = 0;
    CST816S touch_;  // 替换为CST816触摸对象
};

}  // namespace hd_device
}  // namespace esphome
