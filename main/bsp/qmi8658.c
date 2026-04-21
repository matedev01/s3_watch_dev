#include "qmi8658.h"
#include "pins.h"
#include "bsp.h"
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "driver/i2c_master.h"
#include <string.h>

#define TAG "qmi8658"

#define REG_WHO_AM_I   0x00    /* expect 0x05 */
#define REG_CTRL1      0x02
#define REG_CTRL2      0x03    /* accel range/ODR */
#define REG_CTRL3      0x04    /* gyro range/ODR */
#define REG_CTRL7      0x08    /* enable sensors */
#define REG_TEMP_L     0x33
#define REG_AX_L       0x35

static i2c_master_dev_handle_t s_dev = NULL;

/* Scales chosen below: accel ±4g, gyro ±512 dps */
static const float ACC_LSB  = 4.0f  / 32768.0f;
static const float GYRO_LSB = 512.0f / 32768.0f;

static esp_err_t wr(uint8_t reg, uint8_t v)
{
    uint8_t b[2] = { reg, v };
    return i2c_master_transmit(s_dev, b, 2, pdMS_TO_TICKS(50));
}
static esp_err_t rd(uint8_t reg, uint8_t *buf, size_t len)
{
    return i2c_master_transmit_receive(s_dev, &reg, 1, buf, len, pdMS_TO_TICKS(50));
}

esp_err_t qmi8658_init(void)
{
    if (s_dev) return ESP_OK;
    i2c_device_config_t cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address  = BSP_IMU_ADDR,
        .scl_speed_hz    = 400000,
    };
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(bsp_i2c_bus(), &cfg, &s_dev), TAG, "add dev");

    uint8_t who = 0;
    ESP_RETURN_ON_ERROR(rd(REG_WHO_AM_I, &who, 1), TAG, "who");
    if (who != 0x05) {
        ESP_LOGW(TAG, "unexpected WHO_AM_I=0x%02x", who);
    }

    ESP_RETURN_ON_ERROR(wr(REG_CTRL1, 0x60), TAG, "ctrl1");   /* addr auto-inc, big-endian off */
    ESP_RETURN_ON_ERROR(wr(REG_CTRL2, 0x24), TAG, "ctrl2");   /* accel ±4g, ODR 250 Hz */
    ESP_RETURN_ON_ERROR(wr(REG_CTRL3, 0x54), TAG, "ctrl3");   /* gyro ±512dps, ODR 250 Hz */
    ESP_RETURN_ON_ERROR(wr(REG_CTRL7, 0x03), TAG, "ctrl7");   /* enable accel + gyro */
    ESP_LOGI(TAG, "QMI8658 ready");
    return ESP_OK;
}

static inline int16_t join_i16(uint8_t lo, uint8_t hi)
{
    return (int16_t)((uint16_t)hi << 8 | lo);
}

esp_err_t qmi8658_read(qmi8658_sample_t *out)
{
    if (!out) return ESP_ERR_INVALID_ARG;
    uint8_t buf[14];
    /* read temp (2) + accel (6) + gyro (6) starting at TEMP_L */
    ESP_RETURN_ON_ERROR(rd(REG_TEMP_L, buf, sizeof(buf)), TAG, "read");

    int16_t t_raw = join_i16(buf[0], buf[1]);
    int16_t ax = join_i16(buf[2],  buf[3]);
    int16_t ay = join_i16(buf[4],  buf[5]);
    int16_t az = join_i16(buf[6],  buf[7]);
    int16_t gx = join_i16(buf[8],  buf[9]);
    int16_t gy = join_i16(buf[10], buf[11]);
    int16_t gz = join_i16(buf[12], buf[13]);

    out->temp_c = t_raw / 256.0f;
    out->ax = ax * ACC_LSB;
    out->ay = ay * ACC_LSB;
    out->az = az * ACC_LSB;
    out->gx = gx * GYRO_LSB;
    out->gy = gy * GYRO_LSB;
    out->gz = gz * GYRO_LSB;
    return ESP_OK;
}
