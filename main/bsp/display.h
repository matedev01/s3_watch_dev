#pragma once

#include "esp_err.h"
#include "esp_lcd_types.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Bring up the ST7701 RGB panel and register it with esp_lvgl_port.
 * On success, returns the LVGL display handle.
 */
lv_disp_t *display_init(void);

/** Raw ESP-LCD panel handle (useful for power-down hooks). */
esp_lcd_panel_handle_t display_panel(void);

#ifdef __cplusplus
}
#endif
