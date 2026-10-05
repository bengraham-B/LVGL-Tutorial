// ============================================================
// Libraries
// ============================================================
#include <Arduino.h>   // Arduino core: setup(), loop(), Serial, delay()
#include <lvgl.h>      // LVGL graphics library: every lv_* function
#include <TFT_eSPI.h>  // Low-level SPI driver for the ILI9341 (CYD display)
#include <XPT2046_Touchscreen.h>
// ============================================================
// Global objects and buffers
// ============================================================
TFT_eSPI tft;  // The physical display driver instance

// LVGL's render buffer. LVGL draws into this RAM area, then
// my_disp_flush() pushes it to the screen over SPI.
// 320 * 10 = 320 px wide × 10 px tall = one horizontal strip.
// Using a small partial buffer saves RAM (ESP32 has ~320 KB SRAM,
// but LVGL with full-screen double-buffering would need ~150 KB).
static lv_color_t buf[320 * 10];

// CYD touchscreen pins
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

SPIClass touchscreenSPI = SPIClass(VSPI);  // VSPI — critical for CYD
XPT2046_Touchscreen touchscreen(XPT2046_CS);
// LVGL touch read callback — called by LVGL every frame
void my_touch_read(lv_indev_t *indev, lv_indev_data_t *data) {
    if (touchscreen.tirqTouched() && touchscreen.touched()) {
        TS_Point p = touchscreen.getPoint();

        // These calibration values work for most CYDs.
        // If touch is inverted/mirrored, swap or adjust these.
        int16_t x = map(p.x, 200, 3700, 0, 320);
        int16_t y = map(p.y, 240, 3800, 0, 240);

        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

// ============================================================
// Display flush callback — the LVGL ↔ TFT bridge
// ============================================================
// LVGL calls this every time it has rendered a rectangle of pixels.
// We take that pixel data and hand it to TFT_eSPI, which pushes it
// over SPI to the ILI9341 panel. You basically never edit this.
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    uint32_t w = area->x2 - area->x1 + 1;   // width of the dirty rect
    uint32_t h = area->y2 - area->y1 + 1;   // height of the dirty rect

    tft.startWrite();                                    // grab SPI bus
    tft.setAddrWindow(area->x1, area->y1, w, h);         // set the draw window
    tft.pushColors((uint16_t *)px_map, w * h, true);     // blast pixels (swap bytes)
    tft.endWrite();                                      // release SPI bus

    lv_display_flush_ready(disp);  // tell LVGL "this chunk is on screen, send more"
}

static lv_obj_t * counter_label;
static uint32_t  click_count;




static void event_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    click_count++;
    LV_LOG_USER("Button Clicked (%u)", (unsigned)click_count);
    lv_label_set_text_fmt(counter_label, "Clicks (%u)", (unsigned)click_count);
}

// --------------------------------------------------



// ============================================================
// setup() — runs once at boot
// ============================================================
void setup() {

    // ---- 1. Serial + hardware init ----
    Serial.begin(115200);       // USB serial for debug prints
    delay(500);                 // give the serial port time to come up

    tft.init();                 // wake up the ILI9341
    tft.setRotation(3);         // 1 = landscape 320×240 (CYD default)
    tft.fillScreen(TFT_BLACK);  // clear panel before LVGL owns it

    // ---- 2. LVGL core + display registration ----
    lv_init();                                  // boot LVGL's state machine

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

    // ---- 3. Touchscreen init + LVGL input device ----
    touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    touchscreen.begin(touchscreenSPI);
    touchscreen.setRotation(1);  // match display rotation

    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, my_touch_read);
    lv_indev_set_display(indev, disp);  // link to your display

    // ========================================================
    // ==   YOUR UI STARTS HERE                              ==
    // ========================================================

    // ---- Container (the parent of everything below) ----
    // lv_screen_active() is the root screen object. The container is a
    // child of it and will hold our grid of labels.
    lv_obj_t * container = lv_obj_create(lv_screen_active());

    #pragma region remove_styles
    lv_obj_set_style_pad_all(container, 0, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    #pragma endregion remove_styles



    // ---- Grid template definition ----
    // A grid needs two arrays: one for column widths, one for row heights.
    // Each entry is a pixel value, LV_GRID_FR(n) for a fraction, or
    // LV_GRID_CONTENT for auto-size. LV_GRID_TEMPLATE_LAST terminates it.
    #pragma region Grid_Array
    // 3 columns of 70 px each → total width 210 px
    static const int32_t container_style_grid_column_dsc_array_0[] ={LV_GRID_FR(1), 70, 70, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_style_grid_column_dsc_array(container, container_style_grid_column_dsc_array_0, 0);

    // 2 rows of 44 px each → total height 88 px
    static const int32_t container_style_grid_row_dsc_array_1[] ={44, 44, 44, 65, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_style_grid_row_dsc_array(container,container_style_grid_row_dsc_array_1, 0);
    #pragma endregion Grid_Array

    // Tell the container to actually use the grid layout engine.
    lv_obj_set_style_layout(container, LV_LAYOUT_GRID, 0);

    // Shrink-wrap the container to fit its grid (no extra padding space).
    lv_obj_set_size(container,  lv_pct(100), lv_pct(100));

    // ---------------------------------------------------------
    // Cell placement pattern
    // ---------------------------------------------------------
    // For every widget placed in the grid we set 4 style properties:
    //   grid_cell_column_pos  → which column (0-based)
    //   grid_cell_row_pos     → which row    (0-based)
    //   grid_cell_x_align     → horizontal alignment inside the cell
    //   grid_cell_y_align     → vertical   alignment inside the cell
    // LV_GRID_ALIGN_STRETCH makes the widget fill its entire cell.
    // ---------------------------------------------------------

    // ---- Row 0, Column 0 — blue cell labelled "0,0" ----
    lv_obj_t * label_1 = lv_label_create(container);
    lv_obj_set_style_grid_cell_x_align(label_1, LV_GRID_ALIGN_STRETCH, 0);
    lv_obj_set_style_grid_cell_column_pos(label_1, 0, 0);
    lv_obj_set_style_grid_cell_y_align(label_1, LV_GRID_ALIGN_STRETCH, 0);
    lv_obj_set_style_grid_cell_row_pos(label_1, 0, 0);
    lv_obj_set_style_bg_color(label_1, lv_color_hex(0x4a90d9), 0);       // blue bg
    lv_obj_set_style_bg_opa(label_1, (255 * 100 / 100), 0);              // 100% opaque
    lv_obj_set_style_text_color(label_1, lv_color_hex(0xffffff), 0);     // white text
    lv_label_set_text(label_1, "0,0");

    // ---- Row 0, Column 1 — blue cell labelled "1,0" ----
    lv_obj_t * label_2 = lv_label_create(container);
    lv_obj_set_style_grid_cell_x_align(label_2, LV_GRID_ALIGN_STRETCH, 0);
    lv_obj_set_style_grid_cell_column_pos(label_2, 1, 0);
    lv_obj_set_style_grid_cell_y_align(label_2, LV_GRID_ALIGN_STRETCH, 0);
    lv_obj_set_style_grid_cell_row_pos(label_2, 0, 0);
    lv_obj_set_style_bg_color(label_2, lv_color_hex(0x4a90d9), 0);
    lv_obj_set_style_bg_opa(label_2, (255 * 100 / 100), 0);
    lv_obj_set_style_text_color(label_2, lv_color_hex(0xffffff), 0);
    lv_label_set_text(label_2, "1,0");

    // ---- Row 0, Column 2 — blue cell labelled "2,0" ----
    lv_obj_t * label_3 = lv_label_create(container);
    lv_obj_set_style_grid_cell_x_align(label_3, LV_GRID_ALIGN_STRETCH, 0);
    lv_obj_set_style_grid_cell_column_pos(label_3, 2, 0);
    lv_obj_set_style_grid_cell_y_align(label_3, LV_GRID_ALIGN_STRETCH, 0);
    lv_obj_set_style_grid_cell_row_pos(label_3, 0, 0);
    lv_obj_set_style_bg_color(label_3, lv_color_hex(0x4a90d9), 0);
    lv_obj_set_style_bg_opa(label_3, (255 * 100 / 100), 0);
    lv_obj_set_style_text_color(label_3, lv_color_hex(0xffffff), 0);
    lv_label_set_text(label_3, "2,0");

    // ---- Row 1, Column 0 — green cell labelled "0,1" ----
    lv_obj_t * label_4 = lv_label_create(container);
    lv_obj_set_style_grid_cell_x_align(label_4, LV_GRID_ALIGN_STRETCH, 0);
    lv_obj_set_style_grid_cell_column_pos(label_4, 0, 0);
    lv_obj_set_style_grid_cell_y_align(label_4, LV_GRID_ALIGN_STRETCH, 0);
    lv_obj_set_style_grid_cell_row_pos(label_4, 1, 0);
    lv_obj_set_style_bg_color(label_4, lv_color_hex(0x27ae60), 0);       // green bg
    lv_obj_set_style_bg_opa(label_4, (255 * 100 / 100), 0);
    lv_obj_set_style_text_color(label_4, lv_color_hex(0xffffff), 0);
    lv_label_set_text(label_4, "0,1");

    // ---- Row 1, Column 1 — green cell labelled "1,1" ----
    lv_obj_t * label_5 = lv_label_create(container);
    lv_obj_set_style_grid_cell_x_align(label_5, LV_GRID_ALIGN_STRETCH, 0);
    lv_obj_set_style_grid_cell_column_pos(label_5, 1, 0);
    lv_obj_set_style_grid_cell_y_align(label_5, LV_GRID_ALIGN_STRETCH, 0);
    lv_obj_set_style_grid_cell_row_pos(label_5, 1, 0);
    lv_obj_set_style_bg_color(label_5, lv_color_hex(0x27ae60), 0);
    lv_obj_set_style_bg_opa(label_5, (255 * 100 / 100), 0);
    lv_obj_set_style_text_color(label_5, lv_color_hex(0xffffff), 0);
    lv_label_set_text(label_5, "Padding");
    lv_obj_set_style_pad_all(label_5, 5, 0);


    // ---- Row 1, Column 2 — green cell labelled "2,1" ----
    lv_obj_t * label_6 = lv_label_create(container);
    lv_obj_set_style_grid_cell_x_align(label_6, LV_GRID_ALIGN_STRETCH, 0);
    lv_obj_set_style_grid_cell_column_pos(label_6, 2, 0);
    lv_obj_set_style_grid_cell_y_align(label_6, LV_GRID_ALIGN_STRETCH, 0);
    lv_obj_set_style_grid_cell_row_pos(label_6, 1, 0);
    lv_obj_set_style_bg_color(label_6, lv_color_hex(0x27ae60), 0);
    lv_obj_set_style_bg_opa(label_6, (255 * 100 / 100), 0);
    lv_obj_set_style_text_color(label_6, lv_color_hex(0xffffff), 0);
    lv_label_set_text(label_6, "2,1");

    // ---- Row 3, Column 0 — button ----
    lv_obj_t * button = lv_button_create(container); // attach button to the grid
    lv_obj_t * btn_label = lv_label_create(button); // attach label to the button
    lv_label_set_text(btn_label, "Button");
    lv_obj_set_style_grid_cell_column_pos(button, 0, 0);
    lv_obj_set_style_grid_cell_row_pos(button, 3, 0);
    lv_obj_add_event_cb(button, event_cb, LV_EVENT_CLICKED, NULL);

    // ---- Row 3, Column 1 - Counter ----
    counter_label = lv_label_create(container); // Attaching the label to the grid
    lv_obj_set_style_grid_cell_column_pos(counter_label, 1, 0);
    lv_obj_set_style_grid_cell_row_pos(counter_label, 3, 0);
    lv_label_set_text(counter_label, "Clicks: 0");

    // ---- Row 4, Column 1 - Something ----
    std::string value = "GOOSE_CPT";
    lv_obj_t * value_label = lv_label_create(container); // Creating Label object and attaching it to the grid
    lv_label_set_text(value_label, value.c_str()); // setting the text of the label
    lv_obj_set_style_grid_cell_column_pos(value_label, 2, 0); // setting column position of the value
    lv_obj_set_style_grid_cell_row_pos(value_label, 3, 0);

    // ========================================================
    // ==   YOUR UI ENDS HERE                                ==
    // ========================================================
}

// ============================================================
// loop() — runs forever after setup()
// ============================================================
void loop() {
    // Let LVGL do its work: process input, run animations,
    // trigger callbacks, redraw dirty areas.
    // Remove this and nothing on screen will ever update.
    lv_timer_handler();

    // Tiny breather — keeps the CPU from being pegged at 100%.
    delay(5);
}