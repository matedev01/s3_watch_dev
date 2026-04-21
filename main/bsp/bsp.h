#pragma once

#include "esp_err.h"
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Bring up the I2C master bus shared by touch/RTC/IMU/XL9535.
 * Safe to call more than once; returns the same handle.
 */
esp_err_t bsp_i2c_init(void);

/** Get the shared I2C master bus handle. NULL before bsp_i2c_init(). */
i2c_master_bus_handle_t bsp_i2c_bus(void);

/** Initialize XL9535 expander, panel reset sequence, and backlight pin. */
esp_err_t bsp_board_init(void);

/** Backlight duty (0..100). Uses XL9535 pin as an on/off for now. */
void bsp_backlight_set(uint8_t percent);

#ifdef __cplusplus
}
#endif
