#include "st7701.h"
#include "xl9535.h"
#include "pins.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_rom_sys.h"

#define TAG "st7701"

/*
 * 3-wire SPI over I2C expander is painfully slow, but panel init is one-shot.
 * Each frame: CS low, then a 9-bit word { DC, d7..d0 }, MSB first. DC=0 for
 * command, DC=1 for data. SCL idle high, sampled on rising edge.
 */

static inline void bit_delay(void) { esp_rom_delay_us(2); }

static void spi9_begin(void) { xl9535_digital_write(BSP_EXIO_TFT_CS, 0); bit_delay(); }
static void spi9_end(void)   { bit_delay(); xl9535_digital_write(BSP_EXIO_TFT_CS, 1); }

static void spi9_send(uint8_t dc, uint8_t byte)
{
    /* DC bit first */
    xl9535_digital_write(BSP_EXIO_TFT_SCL, 0);
    xl9535_digital_write(BSP_EXIO_TFT_SDA, dc ? 1 : 0);
    bit_delay();
    xl9535_digital_write(BSP_EXIO_TFT_SCL, 1);
    bit_delay();

    for (int i = 7; i >= 0; --i) {
        xl9535_digital_write(BSP_EXIO_TFT_SCL, 0);
        xl9535_digital_write(BSP_EXIO_TFT_SDA, (byte >> i) & 0x1);
        bit_delay();
        xl9535_digital_write(BSP_EXIO_TFT_SCL, 1);
        bit_delay();
    }
}

static void wr_cmd(uint8_t c) { spi9_begin(); spi9_send(0, c); spi9_end(); }
static void wr_data(uint8_t d){ spi9_begin(); spi9_send(1, d); spi9_end(); }

static void wr_seq(uint8_t cmd, const uint8_t *data, size_t len)
{
    wr_cmd(cmd);
    for (size_t i = 0; i < len; ++i) wr_data(data[i]);
}

/*
 * ST7701 init borrowed from the LilyGO T-RGB reference. The "CMD2 BK0/BK1"
 * blocks unlock vendor registers; values come from the panel maker's datasheet
 * and have been validated for this 2.1" 480x480 module.
 */
esp_err_t st7701_init_panel(void)
{
    /* CMD2 BK0 */
    wr_seq(0xFF, (const uint8_t[]){0x77,0x01,0x00,0x00,0x10}, 5);

    wr_seq(0xC0, (const uint8_t[]){0x3B, 0x00}, 2);
    wr_seq(0xC1, (const uint8_t[]){0x0B, 0x02}, 2);
    wr_seq(0xC2, (const uint8_t[]){0x07, 0x02}, 2);
    wr_seq(0xCC, (const uint8_t[]){0x10}, 1);

    /* Gamma */
    wr_seq(0xB0, (const uint8_t[]){0x00,0x11,0x16,0x0E,0x11,0x06,0x05,0x09,
                                   0x08,0x21,0x06,0x13,0x10,0x29,0x31,0x18}, 16);
    wr_seq(0xB1, (const uint8_t[]){0x00,0x11,0x16,0x0E,0x11,0x07,0x05,0x09,
                                   0x09,0x21,0x05,0x13,0x11,0x2A,0x31,0x18}, 16);

    /* CMD2 BK1 */
    wr_seq(0xFF, (const uint8_t[]){0x77,0x01,0x00,0x00,0x11}, 5);
    wr_seq(0xB0, (const uint8_t[]){0x6D}, 1);  /* VOP */
    wr_seq(0xB1, (const uint8_t[]){0x37}, 1);  /* VCOM */
    wr_seq(0xB2, (const uint8_t[]){0x81}, 1);  /* VGH */
    wr_seq(0xB3, (const uint8_t[]){0x80}, 1);
    wr_seq(0xB5, (const uint8_t[]){0x43}, 1);  /* VGL */
    wr_seq(0xB7, (const uint8_t[]){0x85}, 1);
    wr_seq(0xB8, (const uint8_t[]){0x20}, 1);
    wr_seq(0xC1, (const uint8_t[]){0x78}, 1);
    wr_seq(0xC2, (const uint8_t[]){0x78}, 1);
    wr_seq(0xD0, (const uint8_t[]){0x88}, 1);

    wr_seq(0xE0, (const uint8_t[]){0x00,0x00,0x02}, 3);
    wr_seq(0xE1, (const uint8_t[]){0x03,0xA0,0x00,0x00,0x04,0xA0,0x00,0x00,
                                   0x00,0x20,0x20}, 11);
    wr_seq(0xE2, (const uint8_t[]){0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                   0x00,0x00,0x00,0x00,0x00}, 13);
    wr_seq(0xE3, (const uint8_t[]){0x00,0x00,0x11,0x00}, 4);
    wr_seq(0xE4, (const uint8_t[]){0x22,0x00}, 2);
    wr_seq(0xE5, (const uint8_t[]){0x05,0xEC,0xA0,0xA0,0x07,0xEE,0xA0,0xA0,
                                   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, 16);
    wr_seq(0xE6, (const uint8_t[]){0x00,0x00,0x11,0x00}, 4);
    wr_seq(0xE7, (const uint8_t[]){0x22,0x00}, 2);
    wr_seq(0xE8, (const uint8_t[]){0x06,0xED,0xA0,0xA0,0x08,0xEF,0xA0,0xA0,
                                   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, 16);
    wr_seq(0xEB, (const uint8_t[]){0x00,0x00,0x40,0x40,0x00,0x00,0x00}, 7);
    wr_seq(0xED, (const uint8_t[]){0xFF,0xFF,0xFF,0xBA,0x0A,0xBF,0x45,0xFF,
                                   0xFF,0x54,0xFB,0xA0,0xAB,0xFF,0xFF,0xFF}, 16);
    wr_seq(0xEF, (const uint8_t[]){0x10,0x0D,0x04,0x08,0x3F,0x1F}, 6);

    /* Back to user cmd set, then exit sleep and turn display on */
    wr_seq(0xFF, (const uint8_t[]){0x77,0x01,0x00,0x00,0x00}, 5);
    wr_cmd(0x11);
    vTaskDelay(pdMS_TO_TICKS(120));
    wr_seq(0x3A, (const uint8_t[]){0x66}, 1);  /* 18-bit color in -- we drive 16bpp via the panel anyway */
    wr_cmd(0x29);
    vTaskDelay(pdMS_TO_TICKS(20));

    ESP_LOGI(TAG, "ST7701 init done");
    return ESP_OK;
}
