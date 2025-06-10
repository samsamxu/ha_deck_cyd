#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "LGFX.h"
#include "lvgl.h"
#include "CST816S.h"  // 替换为CST816专用库

// 修改为CST816的I2C引脚定义
#define TOUCH_SDA 21
#define TOUCH_SCL 22
#define TOUCH_INT 36   // 中断引脚（如有需要）
#define TOUCH_RST 33   // 复位引脚（如有需要）

namespace esphome {
namespace hd_device {
namespace cst816s_touchscreen {

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
    CST816S touch_;  // CST816触摸屏对象声明
};

}  // namespace hd_device
}  // namespace esphome
