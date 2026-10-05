// ============================================================
// Libraries
// ============================================================
// Arduino core — gives us setup(), loop(), Serial, delay(), etc.
#include <Arduino.h>
// LVGL — the graphics library. Provides every lv_* function.
#include <lvgl.h>
// TFT_eSPI — low-level driver for the ILI9341 display.
#include <TFT_eSPI.h>

// ============================================================
// Global objects and buffers
// ============================================================
// The TFT driver instance. Talks to the physical display over SPI.
TFT_eSPI tft;

// LVGL's render buffer. LVGL draws into this small RAM area, then
// hands it to my_disp_flush() which pushes it to the screen.
// 320 * 10 = 320 pixels wide × 10 rows = one horizontal strip.
// sizeof(buf) is in bytes and passed to LVGL later.
static lv_color_t buf[320 * 10];

// ============================================================
// Display flush callback
// ============================================================
// LVGL calls this whenever it has finished rendering a rectangle.
// We just forward the pixel data to the TFT.
//
// You almost never need to touch this — it's the bridge between
// LVGL's internal rendering and the physical screen.
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)px_map, w * h, true);
    tft.endWrite();
    lv_display_flush_ready(disp);   // tell LVGL we're done with this chunk
}

// ============================================================
// setup() — runs once at boot
// ============================================================
void setup() {

    // --------------------------------------------------------
    // 1. Serial + hardware init
    // --------------------------------------------------------
    Serial.begin(115200);
    delay(500);                 // small delay so Serial is ready

    tft.init();                 // initialize the ILI9341
    tft.setRotation(3);         // 0=portrait, 1=landscape 320x240, 2/3=flipped
    tft.fillScreen(TFT_BLACK);  // clear the panel before LVGL takes over

    // --------------------------------------------------------
    // 2. LVGL core + display registration
    // --------------------------------------------------------
    lv_init();                  // boot LVGL's internal state machine

    // Create a virtual display of 320x240 and tell LVGL to draw to it.
    lv_display_t *disp = lv_display_create(320, 240);

    // Give LVGL the function it should call to push pixels out.
    lv_display_set_flush_cb(disp, my_disp_flush);

    // Hand over the render buffer. NULL second buffer = single-buffer mode.
    // sizeof(buf) is in bytes; LVGL handles the rest.
    lv_display_set_buffers(disp, buf, NULL, sizeof(buf),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Make this the default display (so lv_screen_active() finds it).
    lv_display_set_default(disp);

    // ========================================================
    // ========================================================
    // ==                                                    ==
    // ==   >>> YOUR CODE STARTS HERE <<<                    ==
    // ==                                                    ==
    // ==   Everything below this point is your UI.         ==
    // ==   Add widgets, styles, callbacks, screens, etc.   ==
    // ==                                                    ==
    // ========================================================
    // ========================================================

    // ---- Example UI: one button with a label ----
    // Create a button as a child of the currently active screen.
    lv_obj_t *btn = lv_button_create(lv_screen_active());

    // Set the button's width and height in pixels.
    lv_obj_set_size(btn, 120, 50);

    // Center it on the parent (the screen).
    lv_obj_center(btn);

    // Create a label as a child of the button.
    lv_obj_t *label = lv_label_create(btn);

    // Set the label's text.
    lv_label_set_text(label, "Hello");

    // Center the label inside the button.
    lv_obj_center(label);

    // ========================================================
    // ==   <<< YOUR CODE ENDS HERE >>>                      ==
    // ========================================================
}

// ============================================================
// loop() — runs forever after setup()
// ============================================================
void loop() {
    // Let LVGL do its work: process input, run animations,
    // trigger callbacks, redraw dirty areas.
    // If you remove this, nothing on screen updates.
    lv_timer_handler();

    // Tiny breather — prevents the loop from hogging the CPU.
    delay(5);

    // --------------------------------------------------------
    // You can also add your own periodic logic here:
    //   - Read sensors
    //   - Check WiFi/MQTT state
    //   - Update a label every N milliseconds
    // Just keep it non-blocking so lv_timer_handler() keeps running.
    // --------------------------------------------------------
}