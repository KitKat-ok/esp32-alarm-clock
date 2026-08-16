#pragma once

#include "../../defines.h"

class OrbitSSD1327 : public Adafruit_SSD1327 {
public:
    int16_t offsetX = 0;
    int16_t offsetY = 0;

    using Adafruit_SSD1327::Adafruit_SSD1327;

    void drawPixel(int16_t x, int16_t y, uint16_t color) override;
};

class OLED_MANAGER {
public:
    bool ScreenEnabled = true;
    bool dimmed = false;

    void initDisplay();
    void display();
    void disable();
    void enable();
    void fadeIn();
    void fadeOut();

    void setOffset(int16_t x, int16_t y);
    void stepOrbit();
    void startOrbitTask();

private:
    uint8_t orbitPhase = 0;
    TaskHandle_t orbitTaskHandle = nullptr;

    static void OrbitTask(void *pvParameters);
};

extern OrbitSSD1327 oled;
extern OLED_MANAGER oledMana;
