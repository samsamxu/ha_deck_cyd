#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "LGFX.h"
#include "lvgl.h"
#include "cst816_touchscreen.h"  // 使用ESPHome自带的CST816驱动

// CST816触摸屏引脚定义
#define TOUCH_SDA 21    // I2C SDA引脚
#define TOUCH_SCL 22    // I2C SCL引脚
#define TOUCH_RST 33    // 复位引脚（-1表示不使用）
#define TOUCH_INT -1    // 中断引脚（-1表示不使用）
#define TOUCH_ROTATION 0 // 触摸旋转角度 (0, 90, 180, 270)

namespace esphome {
namespace hd_device {

static const char *const TAG = "HD_DEVICE";

// 声明全局设备指针
extern HaDeckDevice *global_device;

class HaDeckDevice : public Component
{
public:
    void setup() override;
    void loop() override;
    float get_setup_priority() const override;
    uint8_t get_brightness();
    void set_brightness(uint8_t value);
    
    // 使用ESPHome自带的CST816触摸对象
    cst816::CST816Touchscreen touch_;
    
private:
    unsigned long time_ = 0;
    uint8_t brightness_ = 0;
};

}  // namespace hd_device
}  // namespace esphome
