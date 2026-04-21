#pragma once

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Build all screens and show the watchface. Must be called under lvgl_port_lock(). */
void ui_init(void);

/* Screen accessors */
lv_obj_t *ui_screen_watchface(void);
lv_obj_t *ui_screen_menu(void);
lv_obj_t *ui_screen_settings(void);
lv_obj_t *ui_screen_compass(void);

/* Individual screen builders (populate the given screen object) */
void ui_watchface_build(lv_obj_t *scr);
void ui_menu_build(lv_obj_t *scr);
void ui_settings_build(lv_obj_t *scr);
void ui_compass_build(lv_obj_t *scr);

/* Called once per second from a periodic LVGL timer -- updates clock labels. */
void ui_tick_1s(void);

#ifdef __cplusplus
}
#endif
