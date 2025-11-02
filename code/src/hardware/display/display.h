#pragma once

#include "../../defines.h"

// OledManager class definition
class OLED_MANAGER
{
public:
    
    void initDisplay();
    void display();
    void disable();
    void enable();
    void fadeIn();
    void fadeOut();
    bool ScreenEnabled;
    bool dimmed;
    bool finishedDisplaying;
    bool scrolling;
};

extern Adafruit_SSD1327 oled;

extern OLED_MANAGER oledMana;
