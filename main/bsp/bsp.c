#include "bsp.h"
#include "pins.h"
#include "xl9535.h"
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "bsp"

static i2c_master_bus_handle_t s_i2c_bus = NULL;

esp_err_t bsp_i2c_init(void)
{
    if (s_i2c_bus) return ESP_OK;

    i2c_master_bus_config_t bus_cfg = {
        .clk_source                   = I2C_CLK_SRC_DEFAULT,
        .i2c_port                     = BSP_I2C_PORT,
        .sda_io_num                   = BSP_I2C_SDA,
        .scl_io_num                   = BSP_I2C_SCL,
        .glitch_ignore_cnt            = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&bus_cfg, &s_i2c_bus), TAG, "i2c bus");
    ESP_LOGI(TAG, "I2C bus up on SDA=%d SCL=%d", BSP_I2C_SDA, BSP_I2C_SCL);
    return ESP_OK;
}

i2c_master_bus_handle_t bsp_i2c_bus(void)
{
    return s_i2c_bus;
}

esp_err_t bsp_board_init(void)
{
    ESP_RETURN_ON_ERROR(bsp_i2c_init(), TAG, "i2c");
    ESP_RETURN_ON_ERROR(xl9535_init(s_i2c_bus, BSP_XL9535_ADDR), TAG, "xl9535");

    /* Configure the panel-control pins on the expander as outputs */
    xl9535_pin_mode(BSP_EXIO_TFT_CS,  true);
    xl9535_pin_mode(BSP_EXIO_TFT_SCL, true);
    xl9535_pin_mode(BSP_EXIO_TFT_SDA, true);
    xl9535_pin_mode(BSP_EXIO_TFT_RST, true);
    xl9535_pin_mode(BSP_EXIO_LCD_BL,  true);
    xl9535_pin_mode(BSP_EXIO_SD_CS,   true);

    /* Idle states */
    xl9535_digital_write(BSP_EXIO_TFT_CS,  1);
    xl9535_digital_write(BSP_EXIO_TFT_SCL, 1);
    xl9535_digital_write(BSP_EXIO_TFT_SDA, 1);
    xl9535_digital_write(BSP_EXIO_SD_CS,   1);
    xl9535_digital_write(BSP_EXIO_LCD_BL,  0);

    /* Hold-and-release reset for the TFT */
    xl9535_digital_write(BSP_EXIO_TFT_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    xl9535_digital_write(BSP_EXIO_TFT_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(120));

    return ESP_OK;
}

void bsp_backlight_set(uint8_t percent)
{
    /* Simple on/off until we wire PWM; anything > 0 lights it. */
    xl9535_digital_write(BSP_EXIO_LCD_BL, percent > 0);
}

void bsp_i2c_scan(void)
{
    if (!s_i2c_bus) {
        ESP_LOGW(TAG, "i2c scan requested before bus init");
        return;
    }
    ESP_LOGI(TAG, "I2C scan:");
    int found = 0;
    for (uint8_t addr = 0x08; addr <= 0x77; ++addr) {
        if (i2c_master_probe(s_i2c_bus, addr, pdMS_TO_TICKS(20)) == ESP_OK) {
            ESP_LOGI(TAG, "  0x%02X responds", addr);
            ++found;
        }
    }
    ESP_LOGI(TAG, "I2C scan done, %d device(s)", found);
}
