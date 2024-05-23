#ifndef DEFINES_H
#define DEFINES_H

#include "fonts/fonts.h"
#include "weather/weathericons.h"

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TM1637Display.h>
#include <BH1750.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include <Preferences.h>
#include <menu.h>
#include <menuIO/adafruitGfxOut.h>
#include <menuIO/keyIn.h>
#include <menuIO/serialOut.h>
#include <OLED_SSD1306_Chart.h>
#include <OpenWeather.h>
#include <OneWire.h>
#include <DallasTemperature.h>


extern bool OTAEnabled;

// Oled display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SSD1306_I2C_ADDRESS 0x3C

// Led Display
#define CLK  19 // The ESP32 pin GPIO22 connected to CLK
#define DIO  18 // The ESP32 pin GPIO23 connected to DIO

// NTP
#define TIME_OFFSET_S 3600            // Time offset in seconds, use this as timezones

// Light Sensor
#define OLED_DIM_THRESHOLD 30.0 
#define LED_DIM_THRESHOLD 30.0 

#define MAX_INCREASE_OF_LIGHT_LEVEL 1000

// Menu
#define BUTTON_UP_PIN      GPIO_NUM_12
#define BUTTON_DOWN_PIN    GPIO_NUM_13
#define BUTTON_CONFIRM_PIN GPIO_NUM_14
#define BUTTON_EXIT_PIN    GPIO_NUM_15

#define TOUCH_BUTTON_PIN 4
#define TOUCH_BUTTON_THRESHOLD 28
#define TOUCH_BUTTON_THRESHOLD_ON_BATTERY 40


#define TOTAL_NAV_BUTTONS 4
#define MAX_DEPTH 20

#define fontX 6
//5
#define fontY 12

// Alarm
#define SUNDAY    0
#define MONDAY    1
#define TUESDAY   2
#define WEDNESDAY 3
#define THURSDAY  4
#define FRIDAY    5
#define SATURDAY  6

#define BUZZER_PIN 27

// Battery
#define MIN_VOLTAGE 3.40  // Minimum voltage of LiPo battery
#define MAX_VOLTAGE 4.20  // Maximum voltage of LiPo battery
#define NUM_SAMPLES 10   // How many samples to read to smooth the voltage out

#define VOLTAGE_DIVIDER_PIN 34  // Change this to the actual pin number you've connected the voltage divider to

#define FULLY_CHARGED_PIN 32
#define CHARGING_PIN 33

#define TIMER_WAKUP_TIME 15000 // In miliseconds
#define GPIO_WAKUP_TIME 30000 // In miliseconds
#define SLEEPING_TIME 60 * 1000000 

#define CHARGING_THRESHOLD 1000
#define STANDBY_THRESHOLD 1000

// Temperature
#define TEMP_SENS_PIN 5
#define TEMP_CHART_READINGS 55 // Number of readings to keep

#define TEMP_OFFSET 1.76


#include "hardware/pitches.h"

#include "confidential.h"

#include "hardware/hardware.h"
#include "functions.h"
#include "WiFi/WiFi.h"
#include "NTP/NTP.h"
#include "weather/weather.h"
#include "LedDisplay/LedDisplay.h"
#include "lightSensor/lightSensor.h"
#include "oledDisplay/interface.h"
#include "oledDisplay/mainPage.h"
#include "oledDisplay/charts.h"
#include "oledDisplay/menus.h"
#include "Flash/Flash.h"
#include "alarms/alarms.h"
#include "battery/battery.h"
#include "tempSensor/tempSensor.h"


#endif
