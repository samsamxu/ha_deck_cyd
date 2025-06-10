#include "hd_device_cyd.h"

// 定义全局设备指针
HaDeckDevice *global_device = nullptr;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf = (lv_color_t *)heap_caps_malloc(TFT_HEIGHT * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);

int16_t x = 0;
int16_t y = 0;

LGFX lcd;

lv_disp_t *indev_disp;
lv_group_t *group;

void IRAM_ATTR flush_pixels(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    uint32_t len = w * h;

    lcd.startWrite();                            /* Start new TFT transaction */
    lcd.setAddrWindow(area->x1, area->y1, w, h); /* set the working window */
    lcd.writePixels((uint16_t *)&color_p->full, len, true);
    lcd.endWrite();                              /* terminate TFT transaction */

    lv_disp_flush_ready(disp);
}

// 修改触摸读取函数适配CST816
void IRAM_ATTR touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    if (global_device) {
        // 从ESPHome触摸屏对象获取触摸状态
        auto touches = global_device->touch_.get_touches();
        
        if (!touches.empty()) {
            // 取第一个触摸点
            auto &touch = touches[0];
            
            // 坐标校准（根据实际屏幕方向调整）
            #if TOUCH_ROTATION == 90
                data->point.x = TFT_HEIGHT - touch.y;
                data->point.y = touch.x;
            #elif TOUCH_ROTATION == 180
                data->point.x = TFT_WIDTH - touch.x;
                data->point.y = TFT_HEIGHT - touch.y;
            #elif TOUCH_ROTATION == 270
                data->point.x = touch.y;
                data->point.y = TFT_WIDTH - touch.x;
            #else
                data->point.x = touch.x;
                data->point.y = touch.y;
            #endif
            
            data->state = LV_INDEV_STATE_PR;
            
            // 调试日志
            ESP_LOGD(TAG, "Touch X: %d, Y: %d", data->point.x, data->point.y);
        } else {
            data->state = LV_INDEV_STATE_REL;
        }
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

void HaDeckDevice::setup() {
    // 设置全局设备指针
    global_device = this;
    
    lv_init();
    lv_theme_default_init(NULL, lv_color_hex(0xFFEB3B), lv_color_hex(0xFF7043), 1, LV_FONT_DEFAULT);

    // 配置并初始化CST816触摸屏
    if (TOUCH_RST >= 0) {
        this->touch_.set_reset_pin(new GPIOPin(TOUCH_RST, OUTPUT));
    }
    if (TOUCH_INT >= 0) {
        this->touch_.set_interrupt_pin(new InternalGPIOPin(TOUCH_INT, INPUT_PULLUP));
    }
    
    // 设置I2C总线和地址
    this->touch_.set_i2c_bus(&Wire);
    this->touch_.set_i2c_address(0x15); // CST816默认地址
    
    // 初始化触摸屏
    this->touch_.setup();
    ESP_LOGI(TAG, "CST816 touch initialized");

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

    lv_obj_t * bg_color = lv_obj_create(lv_scr_act());
    lv_obj_set_size(bg_color, 320, 240);
    lv_obj_set_style_border_width(bg_color, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(bg_color, lv_color_hex(0x171717), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_parent(bg_color, lv_scr_act());
}

void HaDeckDevice::loop() {
    lv_timer_handler();
    
    // 更新触摸屏状态
    this->touch_.update_touches();

    // 定期重置触摸控制器防止卡死
    static uint32_t last_reset = 0;
    if (millis() - last_reset > 30000) {  // 每30秒重置一次
        if (TOUCH_RST >= 0) {
            digitalWrite(TOUCH_RST, LOW);
            delay(5);
            digitalWrite(TOUCH_RST, HIGH);
            delay(50);
        }
        last_reset = millis();
        ESP_LOGD(TAG, "Touch controller reset");
    }

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
