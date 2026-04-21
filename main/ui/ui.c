#include "ui.h"
#include "esp_log.h"

#define TAG "ui"

static lv_obj_t *s_scr_watchface;
static lv_obj_t *s_scr_menu;
static lv_obj_t *s_scr_settings;
static lv_obj_t *s_scr_compass;

lv_obj_t *ui_screen_watchface(void) { return s_scr_watchface; }
lv_obj_t *ui_screen_menu(void)      { return s_scr_menu;      }
lv_obj_t *ui_screen_settings(void)  { return s_scr_settings;  }
lv_obj_t *ui_screen_compass(void)   { return s_scr_compass;   }

static void tick_cb(lv_timer_t *t) { (void)t; ui_tick_1s(); }

void ui_init(void)
{
    s_scr_watchface = lv_obj_create(NULL);
    s_scr_menu      = lv_obj_create(NULL);
    s_scr_settings  = lv_obj_create(NULL);
    s_scr_compass   = lv_obj_create(NULL);

    /* Every screen gets a black background -- saves power on the LCD. */
    lv_obj_t *screens[] = { s_scr_watchface, s_scr_menu, s_scr_settings, s_scr_compass };
    for (size_t i = 0; i < sizeof(screens)/sizeof(screens[0]); ++i) {
        lv_obj_set_style_bg_color(screens[i], lv_color_black(), 0);
        lv_obj_set_style_bg_opa(screens[i], LV_OPA_COVER, 0);
        lv_obj_set_style_pad_all(screens[i], 0, 0);
        lv_obj_clear_flag(screens[i], LV_OBJ_FLAG_SCROLLABLE);
    }

    ui_watchface_build(s_scr_watchface);
    ui_menu_build(s_scr_menu);
    ui_settings_build(s_scr_settings);
    ui_compass_build(s_scr_compass);

    lv_scr_load(s_scr_watchface);
    lv_timer_create(tick_cb, 1000, NULL);
    ESP_LOGI(TAG, "UI ready");
}
