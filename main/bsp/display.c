#include "display.h"
#include "pins.h"
#include "st7701.h"
#include "bsp.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lvgl_port.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "display"

static esp_lcd_panel_handle_t s_panel = NULL;

esp_lcd_panel_handle_t display_panel(void) { return s_panel; }

lv_disp_t *display_init(void)
{
    /* Run the ST7701 init sequence before bringing up the RGB interface --
       the panel latches its config from the command stream. */
    ESP_ERROR_CHECK(st7701_init_panel());

    esp_lcd_rgb_panel_config_t panel_cfg = {
        .data_width      = 16,
        .bits_per_pixel  = 16,
        .dma_burst_size  = 64,
        .num_fbs         = 2,   /* double-buffered in PSRAM */
        .clk_src         = LCD_CLK_SRC_DEFAULT,
        .disp_gpio_num   = GPIO_NUM_NC,
        .pclk_gpio_num   = BSP_LCD_PIN_PCLK,
        .vsync_gpio_num  = BSP_LCD_PIN_VSYNC,
        .hsync_gpio_num  = BSP_LCD_PIN_HSYNC,
        .de_gpio_num     = BSP_LCD_PIN_DE,
        .data_gpio_nums  = {
            BSP_LCD_PIN_B0, BSP_LCD_PIN_B1, BSP_LCD_PIN_B2, BSP_LCD_PIN_B3, BSP_LCD_PIN_B4,
            BSP_LCD_PIN_G0, BSP_LCD_PIN_G1, BSP_LCD_PIN_G2, BSP_LCD_PIN_G3, BSP_LCD_PIN_G4, BSP_LCD_PIN_G5,
            BSP_LCD_PIN_R0, BSP_LCD_PIN_R1, BSP_LCD_PIN_R2, BSP_LCD_PIN_R3, BSP_LCD_PIN_R4,
        },
        .timings = {
            .pclk_hz           = BSP_LCD_PCLK_HZ,
            .h_res             = BSP_LCD_H_RES,
            .v_res             = BSP_LCD_V_RES,
            .hsync_back_porch  = BSP_LCD_HSYNC_BACK_PORCH,
            .hsync_front_porch = BSP_LCD_HSYNC_FRONT_PORCH,
            .hsync_pulse_width = BSP_LCD_HSYNC_PULSE_WIDTH,
            .vsync_back_porch  = BSP_LCD_VSYNC_BACK_PORCH,
            .vsync_front_porch = BSP_LCD_VSYNC_FRONT_PORCH,
            .vsync_pulse_width = BSP_LCD_VSYNC_PULSE_WIDTH,
            .flags.pclk_active_neg = false,
        },
        .flags = {
            .fb_in_psram = true,
        },
    };

    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_cfg, &s_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(s_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(s_panel));

    /* Sanity pattern: paint an X-axis red / Y-axis green / diagonal blue gradient
       into FB0 so the colors, pixel order, and PCLK polarity are visually obvious
       before LVGL takes over. Turn the backlight on FIRST -- otherwise the panel
       is dark and the test tells us nothing. */
    bsp_backlight_set(100);
    void *fb0 = NULL;
    if (esp_lcd_rgb_panel_get_frame_buffer(s_panel, 1, &fb0) == ESP_OK && fb0) {
        uint16_t *px = (uint16_t *)fb0;
        for (int y = 0; y < BSP_LCD_V_RES; ++y) {
            for (int x = 0; x < BSP_LCD_H_RES; ++x) {
                uint16_t r = (x * 31) / (BSP_LCD_H_RES - 1);
                uint16_t g = (y * 63) / (BSP_LCD_V_RES - 1);
                uint16_t b = ((x + y) * 31) / (BSP_LCD_H_RES + BSP_LCD_V_RES - 2);
                px[y * BSP_LCD_H_RES + x] = (r << 11) | (g << 5) | b;
            }
        }
        esp_lcd_panel_draw_bitmap(s_panel, 0, 0, BSP_LCD_H_RES, BSP_LCD_V_RES, fb0);
        vTaskDelay(pdMS_TO_TICKS(2500));  /* hold the pattern long enough to see */
    }

    /* Hand the panel off to esp_lvgl_port which owns the flush task + mutex. */
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    const lvgl_port_display_cfg_t disp_cfg = {
        .panel_handle = s_panel,
        .buffer_size  = BSP_LCD_H_RES * 80,
        .double_buffer = true,
        .hres         = BSP_LCD_H_RES,
        .vres         = BSP_LCD_V_RES,
        .monochrome   = false,
        .rotation = {
            .swap_xy  = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma    = false,
            .buff_spiram = true,
        },
    };
    /* num_fbs=2 gives us a true double framebuffer -- no bounce buffer, no tearing.
       bb_mode is the alternative (single FB + bounce) and is mutually exclusive. */
    const lvgl_port_display_rgb_cfg_t rgb_cfg = {
        .flags = {
            .bb_mode       = false,
            .avoid_tearing = true,
        },
    };

    lv_disp_t *disp = lvgl_port_add_disp_rgb(&disp_cfg, &rgb_cfg);
    if (!disp) {
        ESP_LOGE(TAG, "lvgl_port_add_disp_rgb returned NULL");
        return NULL;
    }

    bsp_backlight_set(100);
    ESP_LOGI(TAG, "Display ready: %dx%d", BSP_LCD_H_RES, BSP_LCD_V_RES);
    return disp;
}
