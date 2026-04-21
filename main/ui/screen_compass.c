#include "ui.h"
#include "qmi8658.h"
#include "lvgl.h"
#include <math.h>
#include <stdio.h>

static lv_obj_t *s_tilt_indicator;
static lv_obj_t *s_tilt_label;
static lv_obj_t *s_temp_label;
static lv_timer_t *s_timer;

static void back_cb(lv_event_t *e)
{
    (void)e;
    lv_scr_load_anim(ui_screen_menu(), LV_SCR_LOAD_ANIM_MOVE_RIGHT, 250, 0, false);
}

/* Shows where "up" (pitch/roll) is pointing by moving a dot on a circle.
   Not a real compass -- would need a magnetometer -- but a useful IMU demo. */
static void update_cb(lv_timer_t *t)
{
    (void)t;
    qmi8658_sample_t s;
    if (qmi8658_read(&s) != ESP_OK) return;

    /* Pitch from accel.x, roll from accel.y; clamp to ±1g */
    float pitch = s.ax; if (pitch >  1) pitch =  1; if (pitch < -1) pitch = -1;
    float roll  = s.ay; if (roll  >  1) roll  =  1; if (roll  < -1) roll  = -1;

    lv_coord_t r = 130;  /* drift radius */
    lv_coord_t ox = (lv_coord_t)( roll  * r);
    lv_coord_t oy = (lv_coord_t)(-pitch * r);
    lv_obj_align(s_tilt_indicator, LV_ALIGN_CENTER, ox, oy);

    char buf[48];
    float angle_deg = atan2f(roll, -pitch) * 180.0f / (float)M_PI;
    if (angle_deg < 0) angle_deg += 360.0f;
    snprintf(buf, sizeof(buf), "tilt %.0f°", angle_deg);
    lv_label_set_text(s_tilt_label, buf);

    snprintf(buf, sizeof(buf), "%.1f°C", s.temp_c);
    lv_label_set_text(s_temp_label, buf);
}

static void screen_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SCREEN_LOADED) {
        if (!s_timer) s_timer = lv_timer_create(update_cb, 50, NULL);
    } else if (code == LV_EVENT_SCREEN_UNLOADED) {
        if (s_timer) { lv_timer_del(s_timer); s_timer = NULL; }
    }
}

void ui_compass_build(lv_obj_t *scr)
{
    lv_obj_t *title = lv_label_create(scr);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_label_set_text(title, "Level");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 50);

    /* Outer ring */
    lv_obj_t *ring = lv_obj_create(scr);
    lv_obj_remove_style_all(ring);
    lv_obj_set_size(ring, 300, 300);
    lv_obj_center(ring);
    lv_obj_set_style_radius(ring, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_color(ring, lv_color_hex(0x404040), 0);
    lv_obj_set_style_border_width(ring, 2, 0);
    lv_obj_clear_flag(ring, LV_OBJ_FLAG_SCROLLABLE);

    /* Bubble */
    s_tilt_indicator = lv_obj_create(ring);
    lv_obj_remove_style_all(s_tilt_indicator);
    lv_obj_set_size(s_tilt_indicator, 30, 30);
    lv_obj_set_style_radius(s_tilt_indicator, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(s_tilt_indicator, lv_color_hex(0x40C0FF), 0);
    lv_obj_set_style_bg_opa(s_tilt_indicator, LV_OPA_COVER, 0);
    lv_obj_center(s_tilt_indicator);

    s_tilt_label = lv_label_create(scr);
    lv_obj_set_style_text_font(s_tilt_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_tilt_label, lv_color_hex(0xB0B0B0), 0);
    lv_label_set_text(s_tilt_label, "tilt --°");
    lv_obj_align(s_tilt_label, LV_ALIGN_BOTTOM_MID, 0, -80);

    s_temp_label = lv_label_create(scr);
    lv_obj_set_style_text_font(s_temp_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_temp_label, lv_color_hex(0x808080), 0);
    lv_label_set_text(s_temp_label, "--°C");
    lv_obj_align(s_temp_label, LV_ALIGN_BOTTOM_MID, 0, -60);

    lv_obj_t *back = lv_btn_create(scr);
    lv_obj_set_size(back, 120, 44);
    lv_obj_align(back, LV_ALIGN_BOTTOM_MID, 0, -25);
    lv_obj_set_style_radius(back, 22, 0);
    lv_obj_set_style_bg_color(back, lv_color_hex(0x303030), 0);
    lv_obj_add_event_cb(back, back_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *bl = lv_label_create(back);
    lv_obj_set_style_text_font(bl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(bl, lv_color_white(), 0);
    lv_label_set_text(bl, LV_SYMBOL_LEFT "  Back");
    lv_obj_center(bl);

    lv_obj_add_event_cb(scr, screen_event_cb, LV_EVENT_ALL, NULL);
}
