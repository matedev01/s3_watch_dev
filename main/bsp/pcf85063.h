#pragma once

#include "esp_err.h"
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t pcf85063_init(void);

/** Read current RTC time into `out` (UTC, struct tm semantics). */
esp_err_t pcf85063_get_time(struct tm *out);

/** Write `in` to the RTC. */
esp_err_t pcf85063_set_time(const struct tm *in);

/** Sync the system clock from the RTC (settimeofday). */
esp_err_t pcf85063_sync_system_time(void);

#ifdef __cplusplus
}
#endif
