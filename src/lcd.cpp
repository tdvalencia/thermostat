/* LCD interface for the temperature controller */

#include "lcd.h"

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>

// defining lcd tft display elsewhere
extern MCUFRIEND_kbv tft;

const int TEXTSIZE   = 2;
const int TEXTWIDTH  = TEXTSIZE * 5;
const int TEXTHEIGHT = TEXTSIZE * 8;

void initDisplay(float currentTemp, float setTemp, String changeState) {
    tft.fillScreen(BLACK);

    tft.setCursor(0, 0);
    tft.setTextColor(WHITE); tft.setTextSize(TEXTSIZE);

    // always need a title
    tft.println("thermostat");
    tft.println();

    // current temperature
    String currentTemp_string = "current: " + String(currentTemp) + " C";
    tft.println(currentTemp_string);

    // set temperature
    String setTemp_string = "settemp: " + String(setTemp) + " C";
    tft.println(setTemp_string);
    
    // invariance corresponding to enum values
    String status_string = "status:  " + String(changeState);
    tft.println(status_string);
}

void updateDisplay(float currentTemp, float setTemp, String changeState) {
    int x = 10 * TEXTWIDTH; // or your desired X
    int y = 2 * TEXTHEIGHT; // Y position of the text

    // Clear the old value
    tft.fillRect(x, y, 8 * TEXTWIDTH, 3 * TEXTHEIGHT, BLACK); 

    // Set text properties and print new value
    tft.setTextColor(WHITE); tft.setTextSize(TEXTSIZE);
    tft.setCursor(x, y);
    tft.print(currentTemp);
    tft.setCursor(x, y + TEXTHEIGHT);
    tft.print(setTemp);
    tft.setCursor(x, y + 2 * TEXTHEIGHT);
    tft.print(changeState);
}