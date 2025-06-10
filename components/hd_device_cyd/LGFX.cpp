#include "LGFX.h"

namespace esphome {
namespace hd_device {

LGFX::LGFX(void)
{
  {
    auto cfg = _bus_instance.config();
    cfg.spi_host = HSPI_HOST;
    cfg.freq_write = 40000000;
    cfg.freq_read  = 16000000;
    cfg.spi_3wire  = false;
    cfg.use_lock   = true;
    cfg.dma_channel = SPI_DMA_CH_AUTO;
    cfg.pin_sclk = 14;
    cfg.pin_mosi = 13;
    cfg.pin_miso = 12;
    cfg.pin_dc   = 2;

    _bus_instance.config(cfg);
    _panel_instance.setBus(&_bus_instance);
  }

  {
    auto cfg = _panel_instance.config();

    cfg.pin_cs = 15;
    cfg.pin_rst = -1;
    cfg.pin_busy = -1;
    cfg.panel_width = TFT_WIDTH;
    cfg.panel_height = TFT_HEIGHT;
    cfg.offset_x = 0;
    cfg.offset_y = 0;
    cfg.offset_rotation = 0;
    cfg.dummy_read_pixel = 8;
    cfg.dummy_read_bits = 1;
    cfg.readable = true;
    cfg.invert = true;
    cfg.rgb_order = false;
    cfg.dlen_16bit = false;
    cfg.bus_shared = true;

    _panel_instance.config(cfg);
  }

  {
    auto cfg = _light_instance.config();

    cfg.pin_bl = 27;
    cfg.invert = false;
    cfg.freq = 44100;
    cfg.pwm_channel = 7;

    _light_instance.config(cfg);
    _panel_instance.setLight(&_light_instance);
  }

  {
    auto cfg = _touch_instance.config();

    cfg.x_min = 0;
    cfg.x_max = TFT_WIDTH;
    cfg.y_min = 0;
    cfg.y_max = TFT_HEIGHT;
    cfg.pin_int = -1;
    cfg.bus_shared = false;

    cfg.i2c_port = 0;
    cfg.i2c_addr = 0x15;
    cfg.pin_sda = 33;
    cfg.pin_scl = 32;
    //cfg.freq = 2500000;

    _touch_instance.config(cfg);
    _panel_instance.setTouch(&_touch_instance);
  }
  setPanel(&_panel_instance);
}

}  // namespace ha_deck
}  // namespace esphome
