/*
    3D Print Bed Thermostat
    Written by: Tony Valencia
*/

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <math.h>

#include "lcd.h"

#define MINPRESSURE 1
#define MAXPRESSURE 2000

// screen instantiation  
// all pins are already defined by the MCUFRIEND_kbv library yay
MCUFRIEND_kbv tft;

const int thermPin  = A11;   // Analog Pin for thermosistor
const int heaterPin = 33;    // Digital output for heater (relay)

bool systemOn = false;
double setPoint  = 25.0;      // target temperature 25 degrees C

double high_tolerance = 0.5;       // allowable deviation when heating before toggling off
double low_tolerance = 2.0;        // allowable deviation when cooling before toggling on

/* Orientation and pin inputs */

// ID=0x9341
// Pin configuration depends on screen you are using
// This project used HiLetgo 2.4" ILI9341 240X320 TFT LCD
const int XP=8, XM=A2, YP=A3, YM=9;
const int TS_LEFT=845,TS_RIGHT=166,TS_TOP=877,TS_BOT=126;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

Adafruit_GFX_Button one_up_btn, one_down_btn;
Adafruit_GFX_Button five_up_btn, five_down_btn;
Adafruit_GFX_Button start_stop_btn;

int pixel_x, pixel_y;
bool Touch_getXY(void);

double readThermistor();
double getAverageVout(int pin, int samples);

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
    five_up_btn.initButton(&tft, 280, 190, 75, 100, WHITE, CYAN, BLACK, "+5", 2);
    one_up_btn.initButton(&tft, 200, 190, 75, 100, WHITE, CYAN, BLACK, "+1", 2);
    one_down_btn.initButton(&tft, 120, 190, 75, 100, WHITE, CYAN, BLACK, "-1", 2);
    five_down_btn.initButton(&tft, 40, 190, 75, 100, WHITE, CYAN, BLACK, "-5", 2);
    start_stop_btn.initButton(&tft, 270, 70, 100, 80, WHITE, RED, GREEN, "ON/OFF", 2);
    
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

    double high_threshold = setPoint + high_tolerance;
    double low_threshold = setPoint - low_tolerance;

    if (temperatureC <= low_threshold && systemOn) {
        digitalWrite(heaterPin, HIGH); // Turn on heater
        updateDisplay(temperatureC, setPoint, "HEAT");
    }
    else if (temperatureC >= high_threshold && systemOn) {
        digitalWrite(heaterPin, LOW);  // Turn off heater
        updateDisplay(temperatureC, setPoint, "COOL");
    }
    else if (!systemOn) {
        digitalWrite(heaterPin, LOW);
        updateDisplay(temperatureC, setPoint, "OFF");
    }
    else {
        // inside threshold range
        updateDisplay(temperatureC, setPoint, "STABLE");
    } 

    // Handle touchscreen
    bool down = Touch_getXY();

    one_up_btn.press(down && one_up_btn.contains(pixel_x, pixel_y));
    one_down_btn.press(down && one_down_btn.contains(pixel_x, pixel_y));
    five_up_btn.press(down && five_up_btn.contains(pixel_x, pixel_y));
    five_down_btn.press(down && five_down_btn.contains(pixel_x, pixel_y));
    start_stop_btn.press(down && start_stop_btn.contains(pixel_x, pixel_y));

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
        start_stop_btn.drawButton(!systemOn);
        systemOn = !systemOn;
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
    double Vout = getAverageVout(thermPin, 10); // Convert ADC reading to volts
    double Rfixed = 97000.0; // 100k Ohm resistor measured at 95k

    // Voltage Divider -- where the thermistor is placed matters which Rtherm
    // double Vtherm = Vin - Vout; // Voltage accross resistor for node voltage
    // double Rtherm = Rfixed * ( Vin / Vtherm - 1 ); // higher side of voltage divider
    double Rtherm = Rfixed * Vout / (Vin - Vout); // low side of voltage divider

    // Steinhart-Hart
    double A = -2.1709474284e-01,
        B = -4.0639498149e-01,
        C = -3.3311193490e-01,
        D = 3.7664473196e-01,
        E = -1.2993255594e-01,
        F = 2.3246013377e-02,
        G = -2.4263061424e-03,
        H = 1.4961189332e-04,
        I = -5.0729651684e-06,
        J = 7.3158484762e-08;

    double logRtherm = log(Rtherm);
    double inv_T = A + B*logRtherm + C*pow(logRtherm, 2) + D*pow(logRtherm, 3) + E*pow(logRtherm, 4) 
            + F*pow(logRtherm, 5) + G*pow(logRtherm, 6) + H*pow(logRtherm, 7) + I*pow(logRtherm, 8) + J*pow(logRtherm, 9);
    double tempK = 1.0 / inv_T;

    Serial.print("Vout_Raw:\t"); Serial.print(analogRead(thermPin)); 
    Serial.print("\tVout_GND:\t"); Serial.print(analogRead(A9));
    Serial.print("\tVout_Vcc:\t"); Serial.print(analogRead(A8));
    Serial.print("\tVout:\t"); Serial.print(Vout);
    Serial.print("\tResistance:\t"); Serial.print(Rtherm);
    Serial.print("\tTemperature\t"); Serial.println(tempK - 273.15);

    return tempK - 273.15; // convert to Celsius
}

bool Touch_getXY(void)
{
    // ripped from MCUFRIEND_kbv example
    analogReadResolution(10);
    TSPoint p = ts.getPoint();
    analogReadResolution(12);
    pinMode(YP, OUTPUT);      //restore shared pins
    pinMode(XM, OUTPUT);
    digitalWrite(YP, HIGH);   //because TFT control pins
    digitalWrite(XM, HIGH);
    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
    if (pressed) {
        pixel_x = map(p.y, TS_LEFT, TS_RIGHT, 0, tft.width());
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