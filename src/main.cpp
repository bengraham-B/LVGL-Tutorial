// ============================================================
// Libraries
// ============================================================
#include <Arduino.h>   // Arduino core — setup(), loop(), Serial, delay(), millis()
#include <lvgl.h>      // LVGL graphics library — provides every lv_* function
#include <TFT_eSPI.h>  // Low-level SPI driver for the ILI9341 display

// ============================================================
// Global objects and buffers
// ============================================================
// The TFT driver instance. Talks to the physical display over SPI.
TFT_eSPI tft;

// LVGL's render buffer. LVGL draws into this small RAM area, then
// hands it to my_disp_flush(), which pushes it to the screen.
// 320 * 10 = 320 pixels wide × 10 rows = one horizontal strip.
// Using a partial buffer (instead of full-screen) saves a lot of RAM.
static lv_color_t buf[320 * 10];

// ============================================================
// Subject / Observable
// ============================================================

lv_subject_t global_counter_subject;

lv_subject_t global_sum_counter_subject;

// Called when a subject Value changes
static void counter_observerver_cb(lv_observer_t *observer, lv_subject_t *subject)
{
    int32_t value = lv_subject_get_int(subject);

}

// ============================================================
// Display flush callback — the LVGL ↔ TFT bridge
// ============================================================
// LVGL calls this whenever it has finished rendering a rectangle.
// We take that pixel data and hand it to TFT_eSPI, which pushes it
// over SPI to the ILI9341 panel. You almost never need to edit this.
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    // Width and height of the dirty rectangle LVGL just rendered.
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;

    tft.startWrite();                                // grab the SPI bus
    tft.setAddrWindow(area->x1, area->y1, w, h);     // set the draw window
    tft.pushColors((uint16_t *)px_map, w * h, true); // blast pixels (swap bytes)
    tft.endWrite();                                  // release the SPI bus

    // Tell LVGL "this chunk is on screen, send the next one".
    // Without this call, LVGL stops sending further updates.
    lv_display_flush_ready(disp);
}

// ============================================================
// Globals for the timer-driven label
// ============================================================
// NOTE: these are just POINTERS. No LVGL objects are created here.
// LVGL objects must only be created after lv_init() runs — i.e.
// inside setup(). Creating them at file scope (static init time)
// runs before lv_init() and is undefined behaviour.
lv_obj_t *label_update_timer = nullptr;

// Pointer to the LVGL timer that periodically updates the label.
static lv_timer_t *update_timer = nullptr;

// Called by LVGL every 1000 ms (see lv_timer_create at the bottom
// of setup()). Updates the label with an incrementing counter.
static void update_timer_cb(lv_timer_t *t) {
    // 'static' so the value persists between calls.
    static int counter = 0;

    // lv_label_set_text_fmt() formats directly into LVGL's own
    // buffer — safer than building a std::string temporary and
    // passing a .c_str() pointer that may dangle.
    lv_subject_set_int(&global_sum_counter_subject, counter++);
    lv_label_set_text_fmt(label_update_timer, "%d", counter++);
}

lv_obj_t *label_second_timer = nullptr;
lv_timer_t *second_timer = nullptr;

static void update_timer_cb_second(lv_timer_t *t)
{
    static int counter = 0;
    lv_subject_set_int(&global_counter_subject, counter++);
    lv_subject_set_int(&global_sum_counter_subject, counter++);
    lv_label_set_text_fmt(label_second_timer, "R%d", counter++); // Attaching to Display
}

lv_obj_t *label_third_timer = nullptr;
lv_timer_t *third_timer = nullptr;

static void update_timer_cb_third(lv_timer_t *t)
{
    static int counter = 0;
    lv_subject_set_int(&global_sum_counter_subject, counter++);
    lv_label_set_text_fmt(label_third_timer, "Counter: %d", counter++);
}

lv_obj_t *label_fourth_timer = nullptr;
lv_obj_t *label_global_sum_conter = nullptr;


// ============================================================
// setup() — runs once at boot
// ============================================================
void setup() {

    // --------------------------------------------------------
    // 1. Serial + hardware init
    // --------------------------------------------------------
    Serial.begin(115200);        // USB serial for debug prints
    delay(500);                  // small delay so Serial is ready

    tft.init();                  // wake up the ILI9341
    tft.setRotation(3);          // 320×240 landscape
    tft.fillScreen(TFT_BLACK);   // clear the panel before LVGL takes over

    // --------------------------------------------------------
    // 2. LVGL core + display registration
    // --------------------------------------------------------
    lv_init();                   // boot LVGL's internal state machine


    // ============================================================
    // Initialize Subjects
    // ============================================================

    lv_subject_init_int(&global_counter_subject, 0);
    lv_subject_init_int(&global_sum_counter_subject, 0);

    // *** CRITICAL ***
    // Tell LVGL to use Arduino's millis() as its time base.
    // Without this, lv_tick_get() always returns 0, no timer ever
    // expires, and lv_timer_handler() prints:
    //     "It seems lv_tick_inc() is not called"
    // This is the single line that makes timers (and touch input)
    // actually work on ESP32.
    lv_tick_set_cb((lv_tick_get_cb_t)millis);

    // Create a virtual display of 320×240 and tell LVGL to draw to it.
    lv_display_t *disp = lv_display_create(320, 240);

    // Register the function that pushes pixels to the physical screen.
    lv_display_set_flush_cb(disp, my_disp_flush);

    // Hand over the render buffer. NULL second buffer = single-buffer mode.
    // PARTIAL mode means LVGL renders in chunks (here, 10-px strips)
    // instead of the whole screen at once — much lighter on RAM.
    lv_display_set_buffers(disp, buf, NULL, sizeof(buf),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Make this the default display, so lv_screen_active() finds it.
    lv_display_set_default(disp);

    // ========================================================
    // ==   YOUR UI STARTS HERE                              ==
    // ========================================================

    // ---- The label that the timer will update ----
    // Created AFTER lv_init() and AFTER the display is registered.
    // Stored in the global pointer so the timer callback can find it.
    label_update_timer = lv_label_create(lv_screen_active());
    lv_label_set_text(label_update_timer, "0");
    lv_obj_align(label_update_timer, LV_ALIGN_TOP_LEFT, 10, 10);

    // ---- Example UI: a button with its own label ----
    // NOTE: the button's label uses a DIFFERENT name (btn_label).
    // This prevents any shadowing or confusion with label_update_timer.
    lv_obj_t *btn = lv_button_create(lv_screen_active());
    lv_obj_set_size(btn, 120, 50);         // width × height in pixels
    lv_obj_center(btn);                    // centre on its parent (the screen)

    lv_obj_t *btn_label = lv_label_create(btn);  // label as a child of the button
    lv_label_set_text(btn_label, "Hello");
    lv_obj_center(btn_label);                    // centre inside the button

    // ---- Start the periodic update timer ----
    // Fires every 1000 ms. Because lv_tick_set_cb(millis) is set above,
    // LVGL now has a working clock and this timer will actually expire.
    update_timer = lv_timer_create(update_timer_cb, 1000, NULL);

    // ---- Second Periodic Timer ----
    label_second_timer = lv_label_create(lv_screen_active());
    lv_label_set_text(label_second_timer, "0");
    lv_obj_align(label_second_timer, LV_ALIGN_TOP_LEFT, 10, 30);
    second_timer = lv_timer_create(update_timer_cb_second, 500, NULL);

    // ---- Third Periodic Timer ---
    label_third_timer = lv_label_create(lv_screen_active());
    lv_label_set_text(label_third_timer, "0");
    lv_obj_align(label_third_timer, LV_ALIGN_TOP_LEFT, 10, 50);
    third_timer = lv_timer_create(update_timer_cb_third, 250, NULL);

    // ---- Fourth Periodic Timer ----
    label_fourth_timer = lv_label_create(lv_screen_active());
    lv_obj_align(label_fourth_timer, LV_ALIGN_BOTTOM_MID, 0, -15);
    lv_label_bind_text(label_fourth_timer, &global_counter_subject, "Global: %d");

    // ---- Fifth Counter - Sum ----
    label_global_sum_conter = lv_label_create(lv_screen_active());
    lv_obj_align(label_global_sum_conter, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_label_bind_text(label_global_sum_conter, &global_sum_counter_subject, "Sum: %d");


    // ========================================================
    // ==   YOUR UI ENDS HERE                                ==
    // ========================================================
}

// ============================================================
// loop() — runs forever after setup()
// ============================================================
void loop() {
    // Let LVGL do its work: process timers, redraw dirty areas,
    // run animations, fire callbacks.
    // If you remove this, nothing on screen updates.
    lv_timer_handler();

    // Tiny breather — keeps the CPU from being pegged at 100%.
    // 5 ms is short enough that the UI stays responsive.
    delay(5);
}