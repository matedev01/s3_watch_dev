#include "xl9535.h"
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include <string.h>

#define TAG "xl9535"

/* XL9535 register set (Port 0 only on T-RGB; EXIO0..7 = Port 0 bits 0..7) */
#define REG_INPUT_P0        0x00
#define REG_OUTPUT_P0       0x02
#define REG_POLARITY_P0     0x04
#define REG_CONFIG_P0       0x06   /* 1 = input, 0 = output */

static i2c_master_dev_handle_t s_dev = NULL;
static uint8_t s_out_shadow   = 0xFF;
static uint8_t s_cfg_shadow   = 0xFF;  /* reset default: all inputs */

static esp_err_t write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    return i2c_master_transmit(s_dev, buf, sizeof(buf), pdMS_TO_TICKS(100));
}

static esp_err_t read_reg(uint8_t reg, uint8_t *val)
{
    return i2c_master_transmit_receive(s_dev, &reg, 1, val, 1, pdMS_TO_TICKS(100));
}

esp_err_t xl9535_init(i2c_master_bus_handle_t bus, uint8_t addr)
{
    if (s_dev) {
        return ESP_OK;
    }
    i2c_device_config_t cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address  = addr,
        .scl_speed_hz    = 400000,
    };
    esp_err_t err = i2c_master_bus_add_device(bus, &cfg, &s_dev);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "add_device failed: %s", esp_err_to_name(err));
        return err;
    }
    /* Make all pins inputs on boot, reset output shadow to 0 */
    s_cfg_shadow = 0xFF;
    s_out_shadow = 0x00;
    ESP_RETURN_ON_ERROR(write_reg(REG_CONFIG_P0, s_cfg_shadow), TAG, "cfg");
    ESP_RETURN_ON_ERROR(write_reg(REG_OUTPUT_P0, s_out_shadow), TAG, "out");
    return ESP_OK;
}

esp_err_t xl9535_pin_mode(uint8_t pin, bool output)
{
    if (pin > 7) return ESP_ERR_INVALID_ARG;
    if (output) s_cfg_shadow &= ~(1U << pin);
    else        s_cfg_shadow |=  (1U << pin);
    return write_reg(REG_CONFIG_P0, s_cfg_shadow);
}

esp_err_t xl9535_digital_write(uint8_t pin, bool level)
{
    if (pin > 7) return ESP_ERR_INVALID_ARG;
    if (level) s_out_shadow |=  (1U << pin);
    else       s_out_shadow &= ~(1U << pin);
    return write_reg(REG_OUTPUT_P0, s_out_shadow);
}

esp_err_t xl9535_digital_read(uint8_t pin, bool *level)
{
    if (pin > 7 || !level) return ESP_ERR_INVALID_ARG;
    uint8_t v = 0;
    esp_err_t err = read_reg(REG_INPUT_P0, &v);
    if (err == ESP_OK) *level = (v >> pin) & 0x1;
    return err;
}
