#include <MCUFRIEND_kbv.h>
#include <Adafruit_GFX.h>
#include <TouchScreen.h>

#define MINPRESSURE 1
#define MAXPRESSURE 1000

#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// Touchscreen pin configuration (matches your setup)
const int XP=8, XM=A2, YP=A3, YM=9;

// Use library's constructor for touchscreen
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// Display object
MCUFRIEND_kbv tft;

// Calibration values (may need adjusting — see notes below)
const int TS_LEFT = 118;
const int TS_RT   = 878;
const int TS_TOP  = 235;
const int TS_BOT  = 844;

int pixel_x, pixel_y;

void setup() {
  Serial.begin(9600);
  uint16_t ID = tft.readID();
  Serial.print("TFT ID = 0x");
  Serial.println(ID, HEX);
  tft.begin(ID);
  tft.setRotation(3);
  tft.fillScreen(BLACK);
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(10, 10);
  tft.print("Touchscreen Test");
}

void loop() {
  TSPoint p = ts.getPoint();

  // Restore shared pins for TFT after reading touch
  pinMode(YP, OUTPUT);
  pinMode(XM, OUTPUT);
  digitalWrite(YP, HIGH);
  digitalWrite(XM, HIGH);

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    Serial.print("X = "); Serial.print(p.x);
    Serial.print(" Y = "); Serial.print(p.y);
    Serial.print(" Z = "); Serial.println(p.z);

    // Map touchscreen values to screen pixels
    pixel_x = map(p.y, TS_TOP, TS_BOT, 0, tft.width());
    pixel_y = map(p.x, TS_LEFT, TS_RT, 0, tft.height());

    // Draw small circle where touched
    tft.fillCircle(pixel_x, pixel_y, 3, GREEN);
  }

  delay(50);
}
