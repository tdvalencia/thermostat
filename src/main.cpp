/*
    3D Printer Temperature Controller
    Written by: Tony Valencia
*/

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <math.h>

#include "lcd.h"

#define MINPRESSURE 100
#define MAXPRESSURE 2000

// screen instantiation  
// all pins are already defined by the MCUFRIEND_kbv library yay
MCUFRIEND_kbv tft;

const int thermPin  = A11;   // Analog Pin for thermosistor
const int heaterPin = 33;    // Digital output for heater (relay)
const int analogResolution = 12;

float setPoint  = 25.0;      // target temperature 25 degrees C
float tolerance = 0.5;       // allowable deviation before toggling

// Orientation
const int XP = 6, XM = A2, YP = A1, YM = 7; // ID=0x9341
const int TS_LEFT = 878, TS_RT = 118, TS_TOP = 844, TS_BOT = 235;

// Touchscreen GFX
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

Adafruit_GFX_Button up_btn, down_btn;
int pixel_x, pixel_y;

bool Touch_getXY(void);
float readThermistor();
float getAverageVout(int pin, int samples);

// debug
void debugHeater();
void debugThermistor();

/* START ARDUINO EXECUTION */
void setup() {
    Serial.begin(9600);

    uint16_t ID = tft.readID();
    Serial.print("ID = 0x");
    Serial.println(ID, HEX);
    tft.begin(ID);
    tft.setRotation(3);

    pinMode(heaterPin, OUTPUT);
    analogReadResolution(analogResolution);
    digitalWrite(heaterPin, LOW); // Heater off initially

    delay(500);

    initDisplay(readThermistor(), setPoint, "COOL");

    // intantiate buttons
    up_btn.initButton(&tft,  60, 200, 100, 40, WHITE, CYAN, BLACK, "INC", 2);
    down_btn.initButton(&tft, 180, 200, 100, 40, WHITE, CYAN, BLACK, "DEC", 2);
    up_btn.drawButton(false);
    down_btn.drawButton(false);
}

void loop() {
    float temperatureC = readThermistor(); // have temperature in celcius

    // Bang-Bang Control Logic
    // thermostat state indicator
    if (temperatureC < setPoint - tolerance) {
        digitalWrite(heaterPin, HIGH); // Turn on heater
        updateDisplay(temperatureC, setPoint, "HEAT");
    }
    if (temperatureC > setPoint + tolerance) {
        digitalWrite(heaterPin, LOW);  // Turn off heater
        updateDisplay(temperatureC, setPoint, "COOL");
    }

    // debugHeater(temperatureC);

    // Handle touchscreen
    bool down = Touch_getXY();
    up_btn.press(down && up_btn.contains(pixel_x, pixel_y));
    down_btn.press(down && down_btn.contains(pixel_x, pixel_y));

    if (up_btn.justReleased()) {
        up_btn.drawButton();
    }
    if (down_btn.justReleased()) {
        down_btn.drawButton();
    }
    if (up_btn.justPressed()) {
        up_btn.drawButton(true);
        setPoint++;
    }
    if (down_btn.justPressed()) {
        down_btn.drawButton(true);
        setPoint--;
    }

    delay(5);
}
/* END ARDUINO EXECUTION */

/*
    Functions
*/
float readThermistor() {
    // return temperature in C from thermosistor sensor
    float Vin = 4.86; // actual measured input voltage
    float Vout = getAverageVout(thermPin, 1); // Convert ADC reading to volts
    float Rfixed = 95000.0; // 100k Ohm resistor measured at 95k

    Serial.print("Vout_Raw:\t"); Serial.print(analogRead(thermPin)); 
    Serial.print("\tVout_GND:\t"); Serial.print(analogRead(A9));
    Serial.print("\tVout_Vcc:\t"); Serial.print(analogRead(A8));
    Serial.print("\tVout:\t"); Serial.print(Vout);

    // Voltage Divider
    float Rtherm = Rfixed * ( Vin / Vout - 1 );
    // float Rtherm = Rfixed * Vout / (Vin - Vout); // low side of voltage divider

    Serial.print("\tResistance:\t"); Serial.println(Rtherm);

    // Steinhart-Hart
    float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
    float logRtherm = log(Rtherm);
    float temperature = (1.0 / (c1 + c2*logRtherm + c3*logRtherm*logRtherm*logRtherm));
    float temperatureC = temperature - 273.15;

    return temperatureC;
}

bool Touch_getXY(void) { 
    TSPoint p = ts.getPoint();
    pinMode(YP, OUTPUT);      //restore shared pins
    pinMode(XM, OUTPUT);
    digitalWrite(YP, HIGH);   //because TFT control pins
    digitalWrite(XM, HIGH);
    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
    if (pressed) {
        Serial.println("TOUCH!");
        pixel_x = map(p.y, TS_TOP, TS_BOT, 0, tft.width()); //.kbv makes sense to me
        pixel_y = map(p.x, TS_LEFT, TS_RT, 0, tft.height());
    }
    return pressed;
}

float getAverageVout(int pin, int samples = 10) {
    // convert ADC analogRead to Volts
    long total = 0;
    for (int i = 0; i < samples; i++) {
        total += analogRead(pin);
        delay(2); // allow settling
    }
    float Vcc = 3.3; // your measured value
    float avg = total / (float)samples;
    return avg * (Vcc / pow(2, analogResolution));
}