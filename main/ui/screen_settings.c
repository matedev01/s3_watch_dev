#include "ui.h"
#include "bsp.h"
#include "lvgl.h"
#include <stdio.h>

static lv_obj_t *s_bl_slider;
static lv_obj_t *s_bl_value;

static void back_cb(lv_event_t *e)
{
    (void)e;
    lv_scr_load_anim(ui_screen_menu(), LV_SCR_LOAD_ANIM_MOVE_RIGHT, 250, 0, false);
}

static void bl_change_cb(lv_event_t *e)
{
    int v = lv_slider_get_value(s_bl_slider);
    char buf[8]; snprintf(buf, sizeof(buf), "%d%%", v);
    lv_label_set_text(s_bl_value, buf);
    bsp_backlight_set((uint8_t)v);
}

void ui_settings_build(lv_obj_t *scr)
{
    lv_obj_t *title = lv_label_create(scr);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_label_set_text(title, "Settings");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 60);

    /* Backlight slider */
    lv_obj_t *lbl = lv_label_create(scr);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xB0B0B0), 0);
    lv_label_set_text(lbl, "Backlight");
    lv_obj_align(lbl, LV_ALIGN_CENTER, 0, -40);

    s_bl_slider = lv_slider_create(scr);
    lv_obj_set_size(s_bl_slider, 280, 16);
    lv_slider_set_range(s_bl_slider, 0, 100);
    lv_slider_set_value(s_bl_slider, 100, LV_ANIM_OFF);
    lv_obj_align(s_bl_slider, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(s_bl_slider, bl_change_cb, LV_EVENT_VALUE_CHANGED, NULL);

    s_bl_value = lv_label_create(scr);
    lv_obj_set_style_text_font(s_bl_value, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_bl_value, lv_color_white(), 0);
    lv_label_set_text(s_bl_value, "100%");
    lv_obj_align(s_bl_value, LV_ALIGN_CENTER, 0, 30);

    /* Back button */
    lv_obj_t *back = lv_btn_create(scr);
    lv_obj_set_size(back, 140, 50);
    lv_obj_align(back, LV_ALIGN_BOTTOM_MID, 0, -60);
    lv_obj_set_style_radius(back, 25, 0);
    lv_obj_set_style_bg_color(back, lv_color_hex(0x303030), 0);
    lv_obj_add_event_cb(back, back_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *bl = lv_label_create(back);
    lv_obj_set_style_text_font(bl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(bl, lv_color_white(), 0);
    lv_label_set_text(bl, LV_SYMBOL_LEFT "  Back");
    lv_obj_center(bl);
}
