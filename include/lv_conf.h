/**
 * @file lv_conf.h
 * LVGL 9.x configuration file
 * Target: ESP32 + TFT_eSPI + ILI9341 touchscreen
 */

/* ============================================================
 * Include guard
 * Prevents this file from being included more than once,
 * which would cause macro re-definition conflicts.
 * ============================================================ */
#ifndef LV_CONF_H
#define LV_CONF_H

/* ============================================================
 * PART 1: Color settings
 * ============================================================ */

/*
 * Defines the default color format LVGL uses internally.
 *
 * Before LVGL 9 you wrote:  #define LV_COLOR_DEPTH 16
 * LVGL 9 renamed it to an enum, but the meaning is identical.
 *
 * LV_COLOR_FORMAT_RGB565 = 16-bit color (5 bits R, 6 bits G, 5 bits B)
 * This is the native format of the ILI9341, so no pixel conversion
 * is needed — fastest possible rendering.
 */
#define LV_COLOR_FORMAT_DEFAULT   LV_COLOR_FORMAT_RGB565

/* ============================================================
 * PART 2: Memory settings
 * LVGL 9 uses a stdlib abstraction layer; you MUST explicitly
 * pick which malloc/string/sprintf implementation to use.
 * ============================================================ */

/*
 * Allocator: use LVGL's own built-in heap.
 *
 * Options:
 *   LV_STDLIB_BUILTIN     -> LVGL manages its own memory pool (best for MCUs)
 *   LV_STDLIB_CLIB        -> use standard C library malloc/free
 *   LV_STDLIB_MICROPYTHON -> for MicroPython
 *   LV_STDLIB_CUSTOM      -> you implement lv_malloc etc. yourself
 */
#define LV_USE_STDLIB_MALLOC    LV_STDLIB_BUILTIN

/*
 * String functions: use LVGL's internal versions.
 * Avoids weird behavior in some embedded libc implementations.
 */
#define LV_USE_STDLIB_STRING    LV_STDLIB_BUILTIN

/*
 * sprintf: use LVGL's internal implementation.
 * Used for logging and formatting text in labels.
 */
#define LV_USE_STDLIB_SPRINTF   LV_STDLIB_BUILTIN

/*
 * Size of LVGL's internal heap, in bytes.
 *
 * 48 KB is enough for a simple UI. If your screens have lots
 * of images, labels, or large fonts you may hit OOM.
 *
 * If you see "lv_mem_alloc: couldn't allocate", increase this.
 */
#define LV_MEM_SIZE             (48U * 1024U)

/* ============================================================
 * PART 3: HAL / hardware abstraction settings
 * ============================================================ */

/*
 * Auto-refresh period, in milliseconds.
 * 30 ms ≈ 33 FPS. Smaller = smoother but uses more CPU.
 */
#define LV_DEF_REFR_PERIOD      30

/*
 * Default DPI (dots per inch).
 * This only affects physical-size calculations like scroll
 * momentum. For most projects 130 is fine.
 */
#define LV_DPI_DEF              130

/* ============================================================
 * PART 4: Logging
 * Very useful while debugging. Turn off in production to
 * save flash space.
 * ============================================================ */

/*
 * Enable the logging system.
 * 1 = on, 0 = off
 */
#define LV_USE_LOG              1

/*
 * Minimum log level to print.
 * Options:
 *   LV_LOG_LEVEL_TRACE   -> most verbose, prints everything
 *   LV_LOG_LEVEL_INFO    -> general info
 *   LV_LOG_LEVEL_WARN    -> only warnings (recommended)
 *   LV_LOG_LEVEL_ERROR   -> only errors
 *   LV_LOG_LEVEL_USER    -> your own custom logs
 *   LV_LOG_LEVEL_NONE    -> disable all output
 */
#define LV_LOG_LEVEL            LV_LOG_LEVEL_WARN

/*
 * Log output function.
 * 1 = use printf (goes to Arduino Serial — you must call Serial.begin)
 * 0 = use your own registered lv_log_register_print_cb()
 */
#define LV_LOG_PRINTF           1

/*
 * Assert when a pointer is NULL.
 * Strongly recommended during development — helps find crashes fast.
 */
#define LV_USE_ASSERT_NULL      1

/*
 * Assert when malloc fails.
 * Tells you immediately when you run out of RAM instead
 * of silently crashing later.
 */
#define LV_USE_ASSERT_MALLOC    1

/* ============================================================
 * PART 5: Performance & memory monitors
 * Overlays FPS and memory usage in a corner. Great for tuning.
 * ============================================================ */

/*
 * Frame-rate monitor.
 * 1 = on (turn off for release builds)
 */
#define LV_USE_PERF_MONITOR     0

/*
 * Memory monitor.
 * 1 = on
 */
#define LV_USE_MEM_MONITOR      0

/* ============================================================
 * PART 6: Widgets
 * LVGL calls every UI element a "widget".
 * Only enabling the widgets you actually use significantly
 * reduces firmware size.
 * ============================================================ */

/*
 * Text label. Used by almost every screen. Keep it on.
 */
#define LV_USE_LABEL            1

/*
 * Button. Also very common.
 */
#define LV_USE_BTN              1

/*
 * You can add more widgets here, for example:
 *
 *   #define LV_USE_SLIDER     1    // slider
 *   #define LV_USE_SWITCH     1    // on/off switch
 *   #define LV_USE_CHART      1    // graph
 *   #define LV_USE_IMAGE      1    // images
 *   #define LV_USE_TEXTAREA   1    // text input box
 *   #define LV_USE_KEYBOARD   1    // virtual keyboard
 *   #define LV_USE_DROPDOWN   1    // dropdown menu
 *
 * Full list in:
 *   .pio/libdeps/esp32dev/lvgl/lv_conf_template.h
 */

/* ============================================================
 * PART 7: Display driver interfaces
 *
 * We use TFT_eSPI with our own flush_cb() in main.cpp,
 * so ALL built-in display drivers must be OFF here.
 * Otherwise they'd conflict with TFT_eSPI on the SPI bus,
 * or even break compilation.
 * ============================================================ */

/*
 * LovyanGFX driver.
 * MUST be 0! This was the source of the earlier
 * "my_display.hpp: No such file" error.
 */
#define LV_USE_LOVYAN_GFX       0

/*
 * Linux framebuffer. Not needed on ESP32.
 */
#define LV_USE_LINUX_FBDEV      0

/*
 * Linux DRM (direct rendering manager). Not needed on ESP32.
 */
#define LV_USE_LINUX_DRM        0

/*
 * SDL2 (desktop simulator).
 * Only enable when running the LVGL PC simulator.
 */
#define LV_USE_SDL              0

/*
 * LVGL 9's built-in MIPI panel drivers.
 * These talk to the panel directly over SPI, which conflicts
 * with using TFT_eSPI. Keep them all off.
 */
#define LV_USE_ILI9341          0   /* our panel, but driven by TFT_eSPI */
#define LV_USE_ST7735           0
#define LV_USE_ST7789           0
#define LV_USE_ST7796           0
#define LV_USE_NV3007           0

/* Only enable these if you drop TFT_eSPI and use LVGL's own
 * MIPI driver instead. */

/* ============================================================
 * PART 8: Fonts
 *
 * LVGL ships with Montserrat but compiles none by default.
 * You must explicitly enable each size you use.
 * ============================================================ */

/*
 * Enable the 14-pixel Montserrat font.
 *
 * Bigger fonts use more flash:
 *   14 -> ~10 KB
 *   20 -> ~20 KB
 *   28 -> ~40 KB
 */
#define LV_FONT_MONTSERRAT_14   1

/*
 * Default font.
 * Used when a widget doesn't specify a font.
 * If you change this to 20, remember to enable
 * LV_FONT_MONTSERRAT_20 above as well.
 */
#define LV_FONT_DEFAULT         &lv_font_montserrat_14

/* ============================================================
 * End of include guard.
 *
 * DO NOT add anything after this line.
 * In particular, do NOT add:
 *     #define LV_USE_LOVYAN_GFX 1
 *     #define LV_LGFX_USER_INCLUDE "my_display.hpp"
 * Those are for LovyanGFX projects and will break a TFT_eSPI build.
 * ============================================================ */
#endif /* LV_CONF_H */