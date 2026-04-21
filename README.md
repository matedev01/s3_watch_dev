# LilyGO T-RGB Smart Watch (ESP-IDF v5.5 + LVGL 8.3)

A smart-watch firmware for the **LilyGO T-RGB 2.1" (480×480 round, ESP32-S3R8)**
built on **ESP-IDF v5.5.4** with **LVGL 8.3** via `espressif/esp_lvgl_port`.

Features:
- Analog + digital watchface on a round 480×480 panel
- Swipe-up app menu (Clock / Level / Settings / About)
- Settings screen with live backlight control
- Level app driven by the onboard QMI8658 IMU
- PCF85063 RTC seeded to the system clock at boot

## Hardware

| Part           | Chip           | Bus                                     |
|----------------|----------------|-----------------------------------------|
| Display        | ST7701 480×480 | 16-bit RGB + 3-wire SPI init (via expander) |
| Touch          | CST820         | I²C 0x15                                |
| I/O expander   | XL9535         | I²C 0x20                                |
| RTC            | PCF85063       | I²C 0x51                                |
| IMU            | QMI8658        | I²C 0x6B                                |

Pin map lives in [main/bsp/pins.h](main/bsp/pins.h). If your T-RGB revision
wires the reset / backlight / SD CS to different expander pins, adjust
`BSP_EXIO_*` there.

## Building

Requires ESP-IDF **v5.5.x** (tested on 5.5.4) with the IDF Component Manager.

```bash
. $IDF_PATH/export.sh
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/ttyACM0 flash monitor
```

First build will download managed components into `managed_components/`:

- `lvgl/lvgl` ~8.3.11
- `espressif/esp_lvgl_port` ~2.3

## Layout

```
Watch/
├─ CMakeLists.txt
├─ sdkconfig.defaults        # PSRAM OPI, 16 MB flash, LVGL tuned for RGB
├─ partitions.csv            # 4M app + 4M storage + 6M assets
└─ main/
   ├─ idf_component.yml      # LVGL deps
   ├─ CMakeLists.txt
   ├─ main.c                 # app_main: bring up BSP -> display -> touch -> UI
   ├─ bsp/
   │  ├─ pins.h              # all GPIO / I²C addresses
   │  ├─ bsp.c/.h            # I²C bus + board init + backlight
   │  ├─ xl9535.c/.h         # I/O expander driver
   │  ├─ st7701.c/.h         # panel init over 3-wire SPI (bit-banged via expander)
   │  ├─ display.c/.h        # esp_lcd RGB panel + esp_lvgl_port wiring
   │  ├─ touch_cst820.c/.h   # touch + LVGL indev
   │  ├─ pcf85063.c/.h       # RTC
   │  └─ qmi8658.c/.h        # IMU
   └─ ui/
      ├─ ui.c/.h             # screens container, 1 Hz tick
      ├─ screen_watchface.c  # analog dial + digital HUD
      ├─ screen_menu.c       # 2×2 tile launcher
      ├─ screen_settings.c   # backlight slider
      └─ screen_compass.c    # IMU tilt bubble
```

## Navigating the UI

- **Swipe up** from the watchface → app menu
- **Swipe down** from the menu → watchface
- Tap a tile to enter an app; **Back** button or right-swipe returns you

## Tuning notes

- `BSP_LCD_PCLK_HZ` starts at 14 MHz. Push to 16 MHz once stable; if you see
  tearing/vertical bars, back off or raise `bb_mode` buffers.
- PSRAM OPI (80 MHz) is required — the double framebuffer is ~460 KB per buffer
  and will not fit in internal SRAM.
- LVGL partial buffer is `H_RES × 80` lines, allocated in PSRAM.

## Extending

- Add a new app: create `main/ui/screen_xxx.c`, register it in
  [main/ui/ui.c](main/ui/ui.c), and wire a tile in
  [main/ui/screen_menu.c](main/ui/screen_menu.c).
- Replace the analog face with a bitmap dial by dropping a PNG into
  `assets/` and using `lv_img_set_src` with an LVGL image descriptor.
