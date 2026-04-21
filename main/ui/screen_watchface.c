#include "ui.h"
#include "lvgl.h"
#include <time.h>
#include <stdio.h>

static lv_obj_t *s_time_label;
static lv_obj_t *s_date_label;
static lv_obj_t *s_hour_hand;
static lv_obj_t *s_min_hand;
static lv_obj_t *s_sec_hand;

static void swipe_up_cb(lv_event_t *e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if (dir == LV_DIR_TOP) {
        lv_scr_load_anim(ui_screen_menu(), LV_SCR_LOAD_ANIM_MOVE_TOP, 250, 0, false);
    }
}

/*
 *  Pivot convention:
 *    - Hand's local pivot  = (width/2, length - width/2)  (near the base, centered on width)
 *    - Object's local center = (width/2, length/2)
 *    - lv_obj_align(LV_ALIGN_CENTER, dx, dy) places local center at (parent_cx + dx, parent_cy + dy)
 *    - To place the pivot at the parent center: dy = -(pivot_y - h/2) = (width - length)/2, dx = 0
 *  At angle = 0 the hand then points toward -y (12 o'clock).
 */
static lv_obj_t *make_hand(lv_obj_t *parent, lv_coord_t length, lv_coord_t width, lv_color_t color)
{
    lv_obj_t *h = lv_obj_create(parent);
    lv_obj_remove_style_all(h);
    lv_obj_set_size(h, width, length);
    lv_obj_set_style_bg_color(h, color, 0);
    lv_obj_set_style_bg_opa(h, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(h, width / 2, 0);
    lv_obj_set_style_transform_pivot_x(h, width / 2, 0);
    lv_obj_set_style_transform_pivot_y(h, length - width / 2, 0);
    lv_obj_align(h, LV_ALIGN_CENTER, 0, (width - length) / 2);
    return h;
}

void ui_watchface_build(lv_obj_t *scr)
{
    /* Analog dial */
    lv_obj_t *dial = lv_obj_create(scr);
    lv_obj_remove_style_all(dial);
    lv_obj_set_size(dial, 460, 460);
    lv_obj_center(dial);
    lv_obj_set_style_radius(dial, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_color(dial, lv_color_hex(0x202020), 0);
    lv_obj_set_style_border_width(dial, 3, 0);
    lv_obj_clear_flag(dial, LV_OBJ_FLAG_SCROLLABLE);

    /* 12 tick marks. Each tick rotates around the dial center; we put the pivot
       OUTSIDE the tick (at local y = RADIUS) so the visible rectangle lands near
       the edge of the dial. Align offset satisfies: local_center_y + dy = pivot_y. */
    const int RADIUS = 218;
    for (int i = 0; i < 12; ++i) {
        lv_obj_t *t = lv_obj_create(dial);
        lv_obj_remove_style_all(t);
        bool cardinal = (i % 3 == 0);
        int w = cardinal ? 6  : 3;
        int L = cardinal ? 22 : 12;
        lv_obj_set_size(t, w, L);
        lv_obj_set_style_bg_color(t, lv_color_white(), 0);
        lv_obj_set_style_bg_opa(t, LV_OPA_COVER, 0);
        lv_obj_set_style_transform_pivot_x(t, w / 2, 0);
        lv_obj_set_style_transform_pivot_y(t, RADIUS, 0);
        lv_obj_align(t, LV_ALIGN_CENTER, 0, L / 2 - RADIUS);
        lv_obj_set_style_transform_angle(t, i * 300, 0);  /* 0.1 deg units, 12 ticks x 30 deg */
    }

    s_hour_hand = make_hand(dial, 110, 10, lv_color_white());
    s_min_hand  = make_hand(dial, 165, 7,  lv_color_white());
    s_sec_hand  = make_hand(dial, 200, 3,  lv_color_hex(0xE04040));

    /* Center cap */
    lv_obj_t *cap = lv_obj_create(dial);
    lv_obj_remove_style_all(cap);
    lv_obj_set_size(cap, 18, 18);
    lv_obj_center(cap);
    lv_obj_set_style_radius(cap, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(cap, lv_color_hex(0xE04040), 0);
    lv_obj_set_style_bg_opa(cap, LV_OPA_COVER, 0);

    /* Digital HUD */
    s_time_label = lv_label_create(scr);
    lv_obj_set_style_text_font(s_time_label, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(s_time_label, lv_color_hex(0xE0E0E0), 0);
    lv_label_set_text(s_time_label, "--:--");
    lv_obj_align(s_time_label, LV_ALIGN_CENTER, 0, 90);

    s_date_label = lv_label_create(scr);
    lv_obj_set_style_text_font(s_date_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_date_label, lv_color_hex(0x808080), 0);
    lv_label_set_text(s_date_label, "");
    lv_obj_align(s_date_label, LV_ALIGN_CENTER, 0, 125);

    /* Swipe up -> menu */
    lv_obj_add_event_cb(scr, swipe_up_cb, LV_EVENT_GESTURE, NULL);
}

void ui_tick_1s(void)
{
    time_t now = time(NULL);
    struct tm tm_now;
    localtime_r(&now, &tm_now);

    /* Digital */
    if (s_time_label) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d", tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);
        lv_label_set_text(s_time_label, buf);
    }
    if (s_date_label) {
        static const char *wd[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
        static const char *mo[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
        char buf[32];
        snprintf(buf, sizeof(buf), "%s %s %d",
                 wd[tm_now.tm_wday % 7], mo[tm_now.tm_mon % 12], tm_now.tm_mday);
        lv_label_set_text(s_date_label, buf);
    }

    /* Analog -- LVGL rotation is in 0.1 degree units */
    int16_t sec_ang  = tm_now.tm_sec * 60;                             /* 360°/60 = 6° per sec */
    int16_t min_ang  = tm_now.tm_min * 60 + tm_now.tm_sec;             /* +1° per sec */
    int16_t hour_ang = (tm_now.tm_hour % 12) * 300 + tm_now.tm_min * 5;/* 30° per hr, 0.5° per min */
    if (s_sec_hand)  lv_obj_set_style_transform_angle(s_sec_hand,  sec_ang,  0);
    if (s_min_hand)  lv_obj_set_style_transform_angle(s_min_hand,  min_ang,  0);
    if (s_hour_hand) lv_obj_set_style_transform_angle(s_hour_hand, hour_ang, 0);
}
