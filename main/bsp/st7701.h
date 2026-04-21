#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Send the ST7701 init command sequence via 3-wire SPI bit-banged over
 * the XL9535 expander (CS, SCL, SDA pins). Must run AFTER bsp_board_init().
 */
esp_err_t st7701_init_panel(void);

#ifdef __cplusplus
}
#endif
