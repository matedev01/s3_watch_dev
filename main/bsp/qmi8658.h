#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float ax, ay, az;   /* g  */
    float gx, gy, gz;   /* dps */
    float temp_c;
} qmi8658_sample_t;

esp_err_t qmi8658_init(void);
esp_err_t qmi8658_read(qmi8658_sample_t *out);

#ifdef __cplusplus
}
#endif
