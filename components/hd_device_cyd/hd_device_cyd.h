#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "LGFX.h"
#include "lvgl.h"

namespace esphome {
namespace hd_device {

class HaDeckDevice : public Component {
 public:
  void setup() override;
  void loop() override;
  float get_setup_priority() const override;

 private:
  unsigned long time_ = 0;
  LGFX lcd;
  lv_disp_draw_buf_t draw_buf;
  lv_color_t *buf;
  lv_group_t *group;
  uint8_t brightness_ = 255;

  static void IRAM_ATTR flush_pixels(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
  static void IRAM_ATTR touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
};

}  // namespace hd_device
}  // namespace esphome
