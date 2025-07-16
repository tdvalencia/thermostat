/* LCD interface for the temperature controller */
#pragma once

#include <Arduino.h>

// Assign human-readable names to some common 16-bit color values:
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

/* 
    display temperature and whether
    rising or falling on LCD Display 
*/
void initDisplay(float currentTemp, float setTemp, String changeState);
void updateDisplay(float currentTemp, float setTemp, String changeState);