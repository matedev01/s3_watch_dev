/*
 * LilyGO T-RGB 2.1" (ESP32-S3R8, 480x480 round ST7701 RGB LCD)
 *
 * Several signals are behind an XL9535 I2C GPIO expander; those are named
 * EXIO_* and live on the expander's 0..7 pin map, not the SoC.
 *
 *  !!! VERIFY THIS PIN MAP AGAINST YOUR UNIT !!!
 *  LilyGO has shipped at least three T-RGB revisions (half-inch, 2.1" round,
 *  and 2.8" square) with different RGB data-lane ordering and different
 *  expander pin assignments. Before first flash, cross-check every value in
 *  this file against the schematic of YOUR specific board revision. The
 *  authoritative source is the LilyGO/T-RGB GitHub repo:
 *      https://github.com/Xinyuan-LilyGO/T-RGB
 *  In particular, confirm which GPIO is VSYNC: on the 2.1" round it is
 *  typically GPIO3 (not GPIO48), and the RGB data lanes below are a best-
 *  guess skeleton, NOT a confirmed map.
 */
#pragma once

#include "driver/i2c_types.h"

/* ---------- RGB panel timing ---------- */
#define BSP_LCD_H_RES               480
#define BSP_LCD_V_RES               480

#define BSP_LCD_PCLK_HZ             (14 * 1000 * 1000)
#define BSP_LCD_HSYNC_BACK_PORCH    30
#define BSP_LCD_HSYNC_FRONT_PORCH   20
#define BSP_LCD_HSYNC_PULSE_WIDTH   8
#define BSP_LCD_VSYNC_BACK_PORCH    30
#define BSP_LCD_VSYNC_FRONT_PORCH   20
#define BSP_LCD_VSYNC_PULSE_WIDTH   8

/* ---------- RGB panel GPIOs (LilyGO T-RGB 2.1" round) ----------
 *
 * On this board GPIO48 is physically wired to BOTH the I2C SCL line AND the
 * LCD G2 (DATA7) line. Only one of them is electrically active at a time:
 *   - Before esp_lcd_panel_init(), GPIO48 is an ordinary I2C pin.
 *   - After the RGB panel starts driving the data bus, I2C on GPIO48 is dead.
 * This means the I2C-attached peripherals (XL9535, touch, RTC, IMU) must all
 * be talked to BEFORE display_init() takes the bus, not after.
 */
#define BSP_LCD_PIN_DE              17
#define BSP_LCD_PIN_VSYNC           3
#define BSP_LCD_PIN_HSYNC           46
#define BSP_LCD_PIN_PCLK            9

/* B0..B4 */
#define BSP_LCD_PIN_B0              10
#define BSP_LCD_PIN_B1              11
#define BSP_LCD_PIN_B2              12
#define BSP_LCD_PIN_B3              13
#define BSP_LCD_PIN_B4              14

/* G0..G5 (note: G2 = GPIO48 is shared with I2C SCL, see banner above) */
#define BSP_LCD_PIN_G0              21
#define BSP_LCD_PIN_G1              47
#define BSP_LCD_PIN_G2              48
#define BSP_LCD_PIN_G3              45
#define BSP_LCD_PIN_G4              38
#define BSP_LCD_PIN_G5              39

/* R0..R4 */
#define BSP_LCD_PIN_R0              40
#define BSP_LCD_PIN_R1              41
#define BSP_LCD_PIN_R2              42
#define BSP_LCD_PIN_R3              2
#define BSP_LCD_PIN_R4              1

/* ---------- Shared I2C bus (XL9535 + touch + RTC + IMU) ---------- */
#define BSP_I2C_PORT                I2C_NUM_0
#define BSP_I2C_SDA                 8
#define BSP_I2C_SCL                 48
#define BSP_I2C_FREQ_HZ             400000

/* ---------- XL9535 I/O expander ---------- */
#define BSP_XL9535_ADDR             0x20

/* Expander pin map (verify against your schematic) */
#define BSP_EXIO_TFT_CS             0
#define BSP_EXIO_TFT_SCL            1
#define BSP_EXIO_TFT_SDA            2
#define BSP_EXIO_TFT_RST            3
#define BSP_EXIO_SD_CS              4
#define BSP_EXIO_LCD_BL             5

/* ---------- Touch (CST820) ---------- */
#define BSP_TP_ADDR                 0x15
#define BSP_TP_INT                  16
/* touch reset is shared with the TFT reset on the expander */

/* ---------- RTC (PCF85063) ---------- */
#define BSP_RTC_ADDR                0x51
#define BSP_RTC_INT                 7

/* ---------- IMU (QMI8658) ---------- */
#define BSP_IMU_ADDR                0x6B
#define BSP_IMU_INT1                18
#define BSP_IMU_INT2                15

/* ---------- Misc ---------- */
#define BSP_BAT_ADC                 4
