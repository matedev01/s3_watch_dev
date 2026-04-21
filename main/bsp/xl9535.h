#pragma once

#include "esp_err.h"
#include "driver/i2c_master.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* XL9535: 16-bit I2C I/O expander, 8 of which are brought out as EXIO0..7 on T-RGB. */

esp_err_t xl9535_init(i2c_master_bus_handle_t bus, uint8_t addr);

/** Configure a single expander pin as output (true) or input (false). */
esp_err_t xl9535_pin_mode(uint8_t pin, bool output);

/** Drive a configured output pin. */
esp_err_t xl9535_digital_write(uint8_t pin, bool level);

/** Read an input pin. */
esp_err_t xl9535_digital_read(uint8_t pin, bool *level);

#ifdef __cplusplus
}
#endif
