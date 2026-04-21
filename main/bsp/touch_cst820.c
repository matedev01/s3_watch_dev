#include "touch_cst820.h"
#include "pins.h"
#include "bsp.h"
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"

#define TAG "cst820"

/* CST820 registers */
#define REG_GESTURE   0x01
#define REG_FINGER    0x02
#define REG_X_HIGH    0x03
#define REG_X_LOW     0x04
#define REG_Y_HIGH    0x05
#define REG_Y_LOW     0x06

static i2c_master_dev_handle_t s_dev = NULL;

static esp_err_t cst820_read(uint8_t reg, uint8_t *buf, size_t len)
{
    return i2c_master_transmit_receive(s_dev, &reg, 1, buf, len, pdMS_TO_TICKS(50));
}

static void touch_read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    (void)drv;
    uint8_t buf[6] = {0};
    if (cst820_read(REG_GESTURE, buf, sizeof(buf)) != ESP_OK) {
        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }
    uint8_t fingers = buf[1] & 0x0F;
    if (fingers == 0) {
        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }
    uint16_t x = ((buf[2] & 0x0F) << 8) | buf[3];
    uint16_t y = ((buf[4] & 0x0F) << 8) | buf[5];
    if (x >= BSP_LCD_H_RES) x = BSP_LCD_H_RES - 1;
    if (y >= BSP_LCD_V_RES) y = BSP_LCD_V_RES - 1;
    data->state  = LV_INDEV_STATE_PRESSED;
    data->point.x = x;
    data->point.y = y;
}

esp_err_t touch_cst820_init(lv_disp_t *disp)
{
    i2c_master_bus_handle_t bus = bsp_i2c_bus();
    if (!bus) return ESP_ERR_INVALID_STATE;

    i2c_device_config_t cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address  = BSP_TP_ADDR,
        .scl_speed_hz    = 400000,
    };
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(bus, &cfg, &s_dev), TAG, "add dev");

    /* INT as input; we poll in the LVGL indev callback rather than wire an ISR. */
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << BSP_TP_INT,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type    = LV_INDEV_TYPE_POINTER;
    indev_drv.disp    = disp;
    indev_drv.read_cb = touch_read_cb;
    lv_indev_drv_register(&indev_drv);

    ESP_LOGI(TAG, "CST820 touch ready");
    return ESP_OK;
}
