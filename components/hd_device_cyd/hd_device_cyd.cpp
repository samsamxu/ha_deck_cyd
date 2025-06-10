#include "hd_device_cyd.h"

namespace esphome {
namespace hd_device {

static const char *const TAG = "HD_DEVICE";
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf = (lv_color_t *)heap_caps_malloc(TFT_HEIGHT * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);

// 定义CST820触摸屏配置
#define CST820_I2C_ADDR 0x15
#define CST820_INT_PIN 12  // 触摸中断引脚（根据实际接线修改）
#define CST820_RST_PIN 13  // 触摸复位引脚（可选）

// 触摸点坐标
static int touch_x = 0;
static int touch_y = 0;
static bool touched = false;

LGFX lcd;
lv_disp_t *indev_disp;
lv_group_t *group;

void IRAM_ATTR flush_pixels(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    uint32_t len = w * h;

    lcd.startWrite();
    lcd.setAddrWindow(area->x1, area->y1, w, h);
    lcd.writePixels((uint16_t *)&color_p->full, len, true);
    lcd.endWrite();

    lv_disp_flush_ready(disp);
}

// 触摸回调函数
static void touch_callback(int16_t x, int16_t y) {
    touch_x = x;
    touch_y = y;
    touched = true;
}

void IRAM_ATTR touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    if (touched) {
        data->point.x = touch_x;
        data->point.y = touch_y;
        data->state = LV_INDEV_STATE_PR;
        touched = false;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

void HaDeckDevice::setup() {
    lv_init();
    lv_theme_default_init(NULL, lv_color_hex(0xFFEB3B), lv_color_hex(0xFF7043), 1, LV_FONT_DEFAULT);

    // 初始化CST820触摸屏
    this->touchscreen_ = new cst816s::CST816STouchscreen();
    
    // 获取I2C总线
    auto i2c_bus = *App.get_i2c_buses().begin();
    
    // 配置触摸屏参数
    this->touchscreen_->set_i2c_bus(i2c_bus);
    this->touchscreen_->set_i2c_address(CST820_I2C_ADDR);
    
    // 使用GPIOOutputPin而不是GPIOPin
    this->touchscreen_->set_interrupt_pin(new GPIOPin(CST820_INT_PIN, INPUT_PULLUP));
    
    // 可选：配置复位引脚
    if (CST820_RST_PIN != -1) {
        this->touchscreen_->set_reset_pin(new GPIOPin(CST820_RST_PIN, OUTPUT));
    }
    
    // 设置触摸回调函数
    this->touchscreen_->register_touch_listener([=](int16_t x, int16_t y) {
        touch_callback(x, y);
    });
    
    // 初始化触摸屏
    this->touchscreen_->setup();

    // 初始化显示屏
    lcd.init();

    lv_disp_draw_buf_init(&draw_buf, buf, NULL, TFT_HEIGHT * 20);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = TFT_WIDTH;
    disp_drv.ver_res = TFT_HEIGHT;
    disp_drv.rotated = 0;
    disp_drv.sw_rotate = 0;
    disp_drv.flush_cb = flush_pixels;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.long_press_time = 1000;
    indev_drv.long_press_repeat_time = 300;
    indev_drv.read_cb = touchpad_read;
    lv_indev_drv_register(&indev_drv);

    group = lv_group_create();
    lv_group_set_default(group);

    lcd.setBrightness(brightness_);
}

void HaDeckDevice::loop() {
    // 处理触摸屏事件
    if (this->touchscreen_) {
        this->touchscreen_->loop();
    }
    
    // 处理LVGL任务
    lv_timer_handler();

    // 定期日志
    unsigned long ms = millis();
    if (ms - time_ > 60000) {
        time_ = ms;
        ESP_LOGCONFIG(TAG, "Free memory: %d bytes", esp_get_free_heap_size());
    }
}

float HaDeckDevice::get_setup_priority() const { return setup_priority::DATA; }

uint8_t HaDeckDevice::get_brightness() {
    return brightness_;
}

void HaDeckDevice::set_brightness(uint8_t value) {
    brightness_ = value;
    lcd.setBrightness(brightness_);
}

}  // namespace hd_device
}  // namespace esphome
