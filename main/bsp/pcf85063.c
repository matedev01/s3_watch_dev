#include "pcf85063.h"
#include "pins.h"
#include "bsp.h"
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "driver/i2c_master.h"
#include <sys/time.h>
#include <string.h>

#define TAG "pcf85063"

#define REG_CTRL1     0x00
#define REG_SECONDS   0x04   /* VL flag + BCD seconds */

static i2c_master_dev_handle_t s_dev = NULL;

static uint8_t bcd2bin(uint8_t v) { return (v >> 4) * 10 + (v & 0x0F); }
static uint8_t bin2bcd(uint8_t v) { return ((v / 10) << 4) | (v % 10); }

static esp_err_t reg_read(uint8_t reg, uint8_t *buf, size_t len)
{
    return i2c_master_transmit_receive(s_dev, &reg, 1, buf, len, pdMS_TO_TICKS(50));
}
static esp_err_t reg_write(uint8_t reg, const uint8_t *buf, size_t len)
{
    uint8_t tx[16];
    if (len + 1 > sizeof(tx)) return ESP_ERR_INVALID_SIZE;
    tx[0] = reg;
    memcpy(&tx[1], buf, len);
    return i2c_master_transmit(s_dev, tx, len + 1, pdMS_TO_TICKS(50));
}

esp_err_t pcf85063_init(void)
{
    if (s_dev) return ESP_OK;
    i2c_device_config_t cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address  = BSP_RTC_ADDR,
        .scl_speed_hz    = 400000,
    };
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(bsp_i2c_bus(), &cfg, &s_dev), TAG, "add dev");

    /* Control1: clear STOP bit so the counter runs, 24h mode, normal cap */
    uint8_t ctrl = 0x00;
    ESP_RETURN_ON_ERROR(reg_write(REG_CTRL1, &ctrl, 1), TAG, "ctrl1");
    ESP_LOGI(TAG, "PCF85063 ready");
    return ESP_OK;
}

esp_err_t pcf85063_get_time(struct tm *out)
{
    if (!out) return ESP_ERR_INVALID_ARG;
    uint8_t r[7];
    ESP_RETURN_ON_ERROR(reg_read(REG_SECONDS, r, sizeof(r)), TAG, "read");

    memset(out, 0, sizeof(*out));
    out->tm_sec  = bcd2bin(r[0] & 0x7F);
    out->tm_min  = bcd2bin(r[1] & 0x7F);
    out->tm_hour = bcd2bin(r[2] & 0x3F);
    out->tm_mday = bcd2bin(r[3] & 0x3F);
    out->tm_wday = r[4] & 0x07;
    out->tm_mon  = bcd2bin(r[5] & 0x1F) - 1;       /* tm_mon: 0..11 */
    out->tm_year = 100 + bcd2bin(r[6]);            /* assume 2000s, tm_year = years since 1900 */
    return ESP_OK;
}

esp_err_t pcf85063_set_time(const struct tm *in)
{
    if (!in) return ESP_ERR_INVALID_ARG;
    uint8_t r[7] = {
        bin2bcd(in->tm_sec)  & 0x7F,
        bin2bcd(in->tm_min)  & 0x7F,
        bin2bcd(in->tm_hour) & 0x3F,
        bin2bcd(in->tm_mday) & 0x3F,
        (uint8_t)(in->tm_wday & 0x07),
        bin2bcd(in->tm_mon + 1) & 0x1F,
        bin2bcd(in->tm_year - 100),
    };
    return reg_write(REG_SECONDS, r, sizeof(r));
}

esp_err_t pcf85063_sync_system_time(void)
{
    struct tm t;
    ESP_RETURN_ON_ERROR(pcf85063_get_time(&t), TAG, "rtc read");
    time_t epoch = mktime(&t);
    struct timeval tv = { .tv_sec = epoch, .tv_usec = 0 };
    if (settimeofday(&tv, NULL) != 0) return ESP_FAIL;
    ESP_LOGI(TAG, "system time set: %04d-%02d-%02d %02d:%02d:%02d",
             t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
    return ESP_OK;
}
