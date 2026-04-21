#pragma once

#include "esp_err.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Bring up CST820 touch controller and register an LVGL indev on `disp`. */
esp_err_t touch_cst820_init(lv_disp_t *disp);

#ifdef __cplusplus
}
#endif
