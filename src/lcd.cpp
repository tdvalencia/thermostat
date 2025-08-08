/* LCD interface for the thermostat */

#include "lcd.h"

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>

// defining lcd tft display elsewhere
extern MCUFRIEND_kbv tft;
extern Adafruit_GFX_Button one_up_btn, one_down_btn;
extern Adafruit_GFX_Button five_up_btn, five_down_btn;
extern Adafruit_GFX_Button on_off_btn;

const int TEXTSIZE   = 2;
const int TEXTWIDTH  = TEXTSIZE * 5;
const int TEXTHEIGHT = TEXTSIZE * 8;

void initDisplay(float currentTemp, float setTemp, String changeState) {
    tft.fillScreen(BLACK);

    tft.setCursor(0, 0);
    tft.setTextColor(WHITE); tft.setTextSize(TEXTSIZE);

    // always need a title
    tft.println("tonyv thermostat (Celsius)");
    tft.println();

    // current temperature
    String currentTemp_string = "actual:  " + String(currentTemp);
    tft.println(currentTemp_string);
    tft.println();  // add a newline

    // set temperature
    String setTemp_string = "target:  " + String(setTemp);
    tft.println(setTemp_string);
    tft.println(); // add a newline
    
    // status of the system
    String status_string = "status:  " + String(changeState);
    tft.println(status_string);
    tft.println(); // add a newline

    // intantiate buttons
    five_up_btn.initButton(&tft, 280, 190, 75, 100, WHITE, CYAN, BLACK, "+5", 2);
    one_up_btn.initButton(&tft, 200, 190, 75, 100, WHITE, CYAN, BLACK, "+1", 2);
    one_down_btn.initButton(&tft, 120, 190, 75, 100, WHITE, CYAN, BLACK, "-1", 2);
    five_down_btn.initButton(&tft, 40, 190, 75, 100, WHITE, CYAN, BLACK, "-5", 2);
    on_off_btn.initButton(&tft, 270, 70, 100, 80, WHITE, RED, GREEN, "ON/OFF", 2);
    
    five_up_btn.drawButton(false);
    one_up_btn.drawButton(false);
    one_down_btn.drawButton(false);
    five_down_btn.drawButton(false);
    on_off_btn.drawButton(false);
}

void updateDisplay(float currentTemp, float setTemp, String changeState) {
    int x = 10 * TEXTWIDTH; // or your desired X
    int y = 2 * TEXTHEIGHT; // Y position of the text

    // Clear the old value
    tft.fillRect(x, y, 10 * TEXTWIDTH, 6 * TEXTHEIGHT, BLACK); 

    // Set text properties
    tft.setTextColor(WHITE); tft.setTextSize(TEXTSIZE);

    // Set cursor at the first update position (right of 'actual')
    // then print new values at every other line
    tft.setCursor(x, y);
    tft.print(currentTemp); tft.println();
    tft.setCursor(x, y + 2 * TEXTHEIGHT);
    tft.print(setTemp); tft.println();
    tft.setCursor(x, y + 4 * TEXTHEIGHT);
    tft.print(changeState); tft.println();
}