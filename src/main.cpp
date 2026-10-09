#include <lvgl.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>

// ===================== Display Configuration =====================
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

TFT_eSPI tft = TFT_eSPI();

// ===================== Touch Configuration (CYD) =====================
#define XPT2046_IRQ   36
#define XPT2046_MOSI  32
#define XPT2046_MISO  39
#define XPT2046_CLK   25
#define XPT2046_CS    33

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

// ===================== Touch Read Callback =====================
void touchscreen_read(lv_indev_t *indev, lv_indev_data_t *data) {
  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    TS_Point p = touchscreen.getPoint();

    int x = map(p.x, 200, 3700, 0, SCREEN_WIDTH);
    int y = map(p.y, 240, 3800, 0, SCREEN_HEIGHT);

    Serial.printf("Raw: X=%d Y=%d | Mapped: X=%d Y=%d\n", p.x, p.y, x, y);

    data->point.x = constrain(x, 0, SCREEN_WIDTH - 1);
    data->point.y = constrain(y, 0, SCREEN_HEIGHT - 1);
    data->state = LV_INDEV_STATE_PRESSED;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

// ===================== Display Flush Callback =====================
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)px_map, w * h, true);
  tft.endWrite();

  lv_display_flush_ready(disp);
}

// ===================== Button Event Callback =====================
static const lv_color_t colors[] = {
  LV_COLOR_MAKE(255, 0, 0),
  LV_COLOR_MAKE(0, 255, 0),
  LV_COLOR_MAKE(0, 0, 255),
  LV_COLOR_MAKE(255, 165, 0),
  LV_COLOR_MAKE(128, 0, 128)
};
#define NUM_COLORS (sizeof(colors) / sizeof(colors[0]))

static void btn_event_cb(lv_event_t *e) {
  Serial.println("cb fired");
  lv_event_code_t code = lv_event_get_code(e);
  Serial.printf("event code: %d\n", code);

  if (code == LV_EVENT_PRESSED) {
    lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);

    uintptr_t idx = (uintptr_t)lv_obj_get_user_data(btn);
    idx = (idx + 1) % NUM_COLORS;

    lv_obj_set_style_bg_color(btn, colors[idx], 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);

    lv_obj_set_user_data(btn, (void *)idx);
    Serial.printf("Button pressed! New color index: %d\n", idx);
  }
}

// ===================== Setup =====================
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("Booting...");

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  lv_init();

  static lv_color_t buf[SCREEN_WIDTH * SCREEN_HEIGHT / 10];
  lv_display_t *disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(1);

  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchscreen_read);

  lv_obj_t *btn = lv_button_create(lv_screen_active());
  lv_obj_set_size(btn, 120, 60);
  lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_user_data(btn, (void *)0);

  lv_obj_remove_style_all(btn);
  lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(btn, colors[0], 0);
  lv_obj_set_style_radius(btn, 8, 0);

  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, "Press Me");
  lv_obj_center(label);

  lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_PRESSED, NULL);

  Serial.println("Setup done.");
}

// ===================== Loop =====================
void loop() {
  lv_timer_handler();
  lv_tick_inc(5);
  delay(5);
}