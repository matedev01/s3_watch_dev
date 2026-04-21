#include "ui.h"
#include "lvgl.h"

static void tile_click_cb(lv_event_t *e)
{
    lv_obj_t *target = (lv_obj_t *)lv_event_get_user_data(e);
    if (target) lv_scr_load_anim(target, LV_SCR_LOAD_ANIM_FADE_IN, 250, 0, false);
}

static void swipe_down_cb(lv_event_t *e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if (dir == LV_DIR_BOTTOM) {
        lv_scr_load_anim(ui_screen_watchface(), LV_SCR_LOAD_ANIM_MOVE_BOTTOM, 250, 0, false);
    }
}

static lv_obj_t *make_tile(lv_obj_t *parent, const char *icon, const char *label,
                           lv_obj_t *target)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 130, 130);
    lv_obj_set_style_radius(btn, 20, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x1A1A1A), 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x303030), LV_STATE_PRESSED);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_add_event_cb(btn, tile_click_cb, LV_EVENT_CLICKED, target);

    lv_obj_t *ic = lv_label_create(btn);
    lv_obj_set_style_text_font(ic, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(ic, lv_color_white(), 0);
    lv_label_set_text(ic, icon);
    lv_obj_align(ic, LV_ALIGN_CENTER, 0, -15);

    lv_obj_t *lb = lv_label_create(btn);
    lv_obj_set_style_text_font(lb, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lb, lv_color_hex(0xB0B0B0), 0);
    lv_label_set_text(lb, label);
    lv_obj_align(lb, LV_ALIGN_BOTTOM_MID, 0, -10);
    return btn;
}

void ui_menu_build(lv_obj_t *scr)
{
    /* Title */
    lv_obj_t *title = lv_label_create(scr);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_label_set_text(title, "Apps");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 40);

    /* Grid container (flex 2x2, centered) */
    lv_obj_t *grid = lv_obj_create(scr);
    lv_obj_remove_style_all(grid);
    lv_obj_set_size(grid, 300, 300);
    lv_obj_center(grid);
    lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(grid, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(grid, 12, 0);
    lv_obj_set_style_pad_column(grid, 12, 0);
    lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);

    make_tile(grid, LV_SYMBOL_HOME,     "Clock",    ui_screen_watchface());
    make_tile(grid, LV_SYMBOL_GPS,      "Level",    ui_screen_compass());
    make_tile(grid, LV_SYMBOL_SETTINGS, "Settings", ui_screen_settings());

    /* Hint + back gesture */
    lv_obj_t *hint = lv_label_create(scr);
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(hint, lv_color_hex(0x606060), 0);
    lv_label_set_text(hint, "swipe down  " LV_SYMBOL_DOWN);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -30);

    lv_obj_add_event_cb(scr, swipe_down_cb, LV_EVENT_GESTURE, NULL);
}
