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

#define MINPRESSURE 10
#define MAXPRESSURE 2000

// screen instantiation  
// all pins are already defined by the MCUFRIEND_kbv library yay
MCUFRIEND_kbv tft;

const int thermPin  = A11;   // Analog Pin for thermosistor
const int heaterPin = 33;    // Digital output for heater (relay)

bool heaterOn = false;
double setPoint  = 25.0;      // target temperature 25 degrees C
double tolerance = 1.0;       // allowable deviation before toggling

// Orientation
const int XP = 6, XM = A2, YP = A1, YM = 7; //ID=0x9341
const int TS_LEFT=187,TS_RIGHT=835,TS_TOP=885,TS_BOT=135;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

Adafruit_GFX_Button one_up_btn, one_down_btn;
Adafruit_GFX_Button five_up_btn, five_down_btn;
Adafruit_GFX_Button start_stop_btn;

int pixel_x, pixel_y;
bool Touch_getXY(void);

double readThermistor();
double getAverageVout(int pin, int samples);

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

    analogReadResolution(12);

    pinMode(heaterPin, OUTPUT);
    digitalWrite(heaterPin, LOW); // Heater off initially

    delay(500);

    initDisplay(readThermistor(), setPoint, "COOL");

    // intantiate buttons
    five_up_btn.initButton(&tft, 60, 120, 50, 50, WHITE, CYAN, BLACK, "+5", 2);
    one_up_btn.initButton(&tft, 130, 120, 50, 50, WHITE, CYAN, BLACK, "+1", 2);
    one_down_btn.initButton(&tft, 200, 120, 50, 50, WHITE, CYAN, BLACK, "-1", 2);
    five_down_btn.initButton(&tft, 270, 120, 50, 50, WHITE, CYAN, BLACK, "-5", 2);
    start_stop_btn.initButton(&tft, 70, 200, 100, 50, WHITE, RED, GREEN, "ON", 2);
    
    five_up_btn.drawButton(false);
    one_up_btn.drawButton(false);
    one_down_btn.drawButton(false);
    five_down_btn.drawButton(false);
    start_stop_btn.drawButton(false);
}

void loop() {
    double temperatureC = readThermistor(); // have temperature in celcius

    // Bang-Bang Control Logic
    // thermostat state indicator
    if (temperatureC < setPoint - tolerance && heaterOn) {
        digitalWrite(heaterPin, HIGH); // Turn on heater
        updateDisplay(temperatureC, setPoint, "HEAT");
    }
    else if (temperatureC > setPoint + tolerance && heaterOn) {
        digitalWrite(heaterPin, LOW);  // Turn off heater
        updateDisplay(temperatureC, setPoint, "COOL");
    }
    else if (!heaterOn) {
        updateDisplay(temperatureC, setPoint, "OFF");
    }


    // Handle touchscreen
    bool down = Touch_getXY();

    one_up_btn.press(down && one_up_btn.contains(pixel_x, pixel_y));
    one_down_btn.press(down && one_down_btn.contains(pixel_x, pixel_y));
    five_up_btn.press(down && five_up_btn.contains(pixel_x, pixel_y));
    five_down_btn.press(down && five_down_btn.contains(pixel_x, pixel_y));
    start_stop_btn.press(down && start_stop_btn.contains(pixel_x, pixel_y));

    // if (down) {
    //     tft.fillCircle(pixel_x, pixel_y, 3, GREEN); // temporary
    // }

    if (one_up_btn.justReleased()) {
        one_up_btn.drawButton();
    }
    if (one_down_btn.justReleased()) {
        one_down_btn.drawButton();
    }
    if (one_up_btn.justPressed()) {
        one_up_btn.drawButton(true);
        setPoint += 1;
    }
    if (one_down_btn.justPressed()) {
        one_down_btn.drawButton(true);
        setPoint -= 1;
    }

    if (five_up_btn.justReleased()) {
        five_up_btn.drawButton();
    }
    if (five_down_btn.justReleased()) {
        five_down_btn.drawButton();
    }
    if (five_up_btn.justPressed()) {
        five_up_btn.drawButton(true);
        setPoint += 5;
    }
    if (five_down_btn.justPressed()) {
        five_down_btn.drawButton(true);
        setPoint -= 5;
    }

    if (start_stop_btn.justPressed()) {
        start_stop_btn.drawButton(!heaterOn);
        heaterOn = !heaterOn;
    }

    delay(2);
}
/* END ARDUINO EXECUTION */

/*
    Functions
*/
double readThermistor() {
    // return temperature in C from thermosistor sensor
    double Vin = 4.86; // actual measured input voltage
    double Vout = getAverageVout(thermPin, 30); // Convert ADC reading to volts
    double Rfixed = 97000.0; // 100k Ohm resistor measured at 95k

    // Voltage Divider
    // double Vtherm = Vin - Vout; // Voltage accross resistor for node voltage
    // double Rtherm = Rfixed * ( Vin / Vtherm - 1 );
    double Rtherm = Rfixed * Vout / (Vin - Vout); // low side of voltage divider

    // Steinhart-Hart
    double A = 3.5137043601e+01, B = -2.2548090240e+01, C = 6.0079813523e+00, D = -8.5076103273e-01,
        E = 6.7526011417e-02, F = -2.8483415639e-03, G = 4.9883793734e-05;

    double logRtherm = log(Rtherm);
    double inv_T = A + B*logRtherm + C*pow(logRtherm, 2) + D*pow(logRtherm, 3) + E*pow(logRtherm, 4) + F*pow(logRtherm, 5) + G*pow(logRtherm, 6);
    double tempK = 1.0 / inv_T;

    Serial.print("Vout_Raw:\t"); Serial.print(analogRead(thermPin)); 
    Serial.print("\tVout_GND:\t"); Serial.print(analogRead(A9));
    Serial.print("\tVout_Vcc:\t"); Serial.print(analogRead(A8));
    Serial.print("\tVout:\t"); Serial.print(Vout);
    Serial.print("\tResistance:\t"); Serial.print(Rtherm);
    Serial.print("\tTemperature\t"); Serial.println(tempK - 273.15);

    return tempK - 273.15; // convert to C
}

bool Touch_getXY(void)
{
    analogReadResolution(10);
    TSPoint p = ts.getPoint();
    analogReadResolution(12);
    pinMode(YP, OUTPUT);      //restore shared pins
    pinMode(XM, OUTPUT);
    digitalWrite(YP, HIGH);   //because TFT control pins
    digitalWrite(XM, HIGH);
    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
    if (pressed) {
        pixel_x = map(p.y, TS_LEFT, TS_RIGHT, 0, tft.width()); //.kbv makes sense to me
        pixel_y = map(p.x, TS_BOT, TS_TOP, 0, tft.height());

        // Serial.print("Raw X:\t"); Serial.print(p.x);
        // Serial.print("\tRaw Y:\t"); Serial.print(p.y);
        // Serial.print("\t|\tPixel X:\t"); Serial.print(pixel_x);
        // Serial.print("\tPixel Y: "); Serial.println(pixel_y);
    }
    return pressed;
}

double getAverageVout(int pin, int samples = 10) {
    // convert ADC analogRead to Volts
    long total = 0;
    for (int i = 0; i < samples; i++) {
        total += analogRead(pin);
        delay(2); // allow settling
    }
    double Vcc = 3.3; // your measured value
    double avg = total / (double)samples;
    return avg * (Vcc / 4095.0);
}