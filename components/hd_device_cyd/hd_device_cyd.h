#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "LGFX.h"
#include "lvgl.h"
// 使用 CST816 库适配 CST820 触摸屏
#include "esphome/components/cst816/touchscreen/cst816_touchscreen.h"  // ESPhome 内置的 CST816S 组件

// 定义 CST820 触摸屏的 I²C 地址和引脚
#define CST820_I2C_ADDR 0x15
#define CST820_INT_PIN 36  // 触摸中断引脚
#define CST820_RST_PIN 33  // 触摸复位引脚（可选）

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
    // 添加触摸屏成员变量
    CST816STouchscreen *touchscreen_ = nullptr;
};

}  // namespace hd_device
}  // namespace esphome
