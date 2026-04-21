#include "bsp/bsp.h"
#include "bsp/display.h"
#include "bsp/touch_cst820.h"
#include "bsp/pcf85063.h"
#include "bsp/qmi8658.h"
#include "ui/ui.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_lvgl_port.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "app"

static void nvs_init_or_erase(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS needs erase (%s)", esp_err_to_name(err));
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

void app_main(void)
{
    ESP_LOGI(TAG, "LilyGO T-RGB smart watch booting");
    nvs_init_or_erase();

    /* ---- Pre-LCD I2C phase ----
     * GPIO48 is shared between I2C SCL and LCD G2 on this board. Do everything
     * I2C-related that we can up front, while SCL still wins the mux, then hand
     * the bus over to the display. */
    ESP_ERROR_CHECK(bsp_board_init());         /* I2C bus + XL9535 + panel reset/backlight */

    bsp_i2c_scan();   /* log every I2C responder so mismatched addresses are obvious */

    if (pcf85063_init() == ESP_OK) {
        pcf85063_sync_system_time();
    } else {
        ESP_LOGW(TAG, "RTC not found; clock starts at epoch");
    }
    if (qmi8658_init() != ESP_OK) {
        ESP_LOGW(TAG, "IMU not found; level app will show zeros");
    }

    /* ---- LCD + LVGL ---- */
    lv_disp_t *disp = display_init();
    if (!disp) {
        ESP_LOGE(TAG, "display_init failed");
        return;
    }

    /* Touch driver is registered post-LCD because LVGL indev needs the display.
       Device registration itself does not drive I2C traffic. */
    if (lvgl_port_lock(0)) {
        ESP_ERROR_CHECK(touch_cst820_init(disp));
        ui_init();
        lvgl_port_unlock();
    }

    ESP_LOGI(TAG, "boot complete, free heap = %u", (unsigned)esp_get_free_heap_size());

    /* app_main can return -- LVGL port owns its own task */
}
