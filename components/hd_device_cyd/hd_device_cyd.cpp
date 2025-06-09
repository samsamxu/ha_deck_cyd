#include "hd_device_cyd.h"

namespace esphome {
namespace hd_device {

static const char *const TAG = "HD_DEVICE";
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf = (lv_color_t *)heap_caps_malloc(TFT_HEIGHT * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);

int x = 0;
int y = 0;

// 移除SPI相关代码
// 添加I2C和CST816对象
TwoWire touchWire = TwoWire(0); // 使用I2C0
CST816S touch_; // 已在头文件中声明

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

void IRAM_ATTR touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    uint8_t gesture = touch_.getGesture();
    
    if (gesture != None) {
        x = touch_.getX();
        y = touch_.getY();
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PR;
        ESP_LOGCONFIG(TAG, "Touch detected - X: %d, Y: %d", x, y);
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

void HaDeckDevice::setup() {
    lv_init();
    lv_theme_default_init(NULL, lv_color_hex(0xFFEB3B), lv_color_hex(0xFF7043), 1, LV_FONT_DEFAULT);

    // 初始化I2C总线
    touchWire.begin(TOUCH_SDA, TOUCH_SCL, 400000); // 400kHz I2C速度
    
    // 初始化CST816触摸屏
    if (!touch_.begin(touchWire, TOUCH_RST, TOUCH_INT)) {
        ESP_LOGE(TAG, "CST816 touch initialization failed!");
    } else {
        ESP_LOGI(TAG, "CST816 touch initialized successfully");
        touch_.setAutoSleep(false); // 禁用自动休眠
    }

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
