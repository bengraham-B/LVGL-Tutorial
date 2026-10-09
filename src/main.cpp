#include <SPI.h>
#include <XPT2046_Touchscreen.h>

// CYD touchscreen pins (separate SPI bus from the display)
#define XPT2046_IRQ   36
#define XPT2046_MOSI  32
#define XPT2046_MISO  39
#define XPT2046_CLK   25
#define XPT2046_CS    33

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

void setup() {
    Serial.begin(115200);
    delay(300);

    touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    touchscreen.begin(touchscreenSPI);
    touchscreen.setRotation(1); // landscape; try 3 if inverted

    Serial.println("Touchscreen ready. Touch the screen...");
}

void loop() {
    if (touchscreen.tirqTouched() && touchscreen.touched()) {
        TS_Point p = touchscreen.getPoint();

        // Map raw values to screen pixels
        int x = map(p.x, 200, 3700, 0, 320);
        int y = map(p.y, 240, 3800, 0, 240);

        x = constrain(x, 0, 319);
        y = constrain(y, 0, 239);

        Serial.printf("Touch: X=%d, Y=%d, Z=%d\n", x, y, p.z);
        delay(50);
    }
}