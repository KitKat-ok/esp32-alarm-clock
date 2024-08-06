#ifndef OLEDMANAGER_H
#define OLEDMANAGER_H

#include "../defines.h"

void oledDisplay();
void oledFadeout();
void oledFadein();

void oledDisable();
void oledEnable();

void startScrollingLeft(uint8_t startPage, uint8_t endPage);
void startScrollingRight(uint8_t startPage, uint8_t endPage);

typedef enum {
    OLED_NO_ACTION,
    OLED_FADE_IN,
    OLED_FADE_OUT,
    OLED_DISPLAY,
    OLED_ENABLE,
    OLED_DISABLE,
    OLED_CUSTOM_COMMAND,
    OLED_SCROLL_LEFT,   // New action for scrolling
    OLED_SCROLL_RIGHT,  // New action for scrolling
    // Add more actions if needed
} Action;

typedef struct {
    Action action;
    uint8_t param1; // Parameter 1 for scrolling
    uint8_t param2; // Parameter 2 for scrolling
} ActionData;

void createOledManagerTask();

void sendOledCustomCommand(uint8_t command);

#endif