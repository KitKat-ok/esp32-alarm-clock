#include "hardware.h"
#include <esp_adc_cal.h>

TM1637Display LedDisplay = TM1637Display(CLK, DIO);
LTR_F216A lightMeter;

Adafruit_SHT4x sht4 = Adafruit_SHT4x();

void initOledDisplay();
void initLedDisplay();
void initLightSensor();
void initBuzzer();
void initButtons();
void initTempSensor();
void LowBattery();

void initHardware()
{
  setCpuFrequencyMhz(80); // stable 160,80,240
  delay(2000);
  Serial.begin(115200);
  analogReadResolution(12);       // Set ADC resolution to 12-bit (0-4095)
  analogSetAttenuation(ADC_11db); // Set attenuation to 11dB (0-3.9V)
  analogSetClockDiv(80);
  adcAttachPin(VOLTAGE_DIVIDER_PIN);
  initButtons();
  touchSetCycles(1000, 1000);
  initBuzzer();
  initOledDisplay();
  initLedDisplay();
  initLightSensor();
  initTempSensor();
  LowBattery();
}

void LowBattery() // Prevents battery voltage from going too low by hybernating needs reset after this happens connecting the charger wont wake it up
{
  float rawVoltage = analogRead(VOLTAGE_DIVIDER_PIN) * (3.3 / 4095.0) + 0.8;
  int chargingState = analogRead(CHARGING_PIN);
  int standbyState = analogRead(FULLY_CHARGED_PIN);
  if (standbyState > 3000 || chargingState > 3000)
  {
    charging = true;
  }
  else
  {
    charging = false;
  }

  if (rawVoltage < 3.40 & charging == false)
  {
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    Serial.print("what where is my battery or is it too low?! Im going to sleep I dont know what is happening byeee and Im going to scream while doing soo");

    pinMode(BUZZER_PIN, OUTPUT);
    ledcSetup(0, 2000, 8);
    ledcAttachPin(BUZZER_PIN, 0);

    LedDisplay.clear();
    display.clearDisplay();
    manager.oledDisplay();

    for (size_t i = 0; i < 10; i++)
    {
      for (int dutyCycle = 0; dutyCycle >= 255; dutyCycle--)
      {
        ledcWrite(0, dutyCycle);
        Serial.println(dutyCycle);
        delay(5);
      }

      for (int dutyCycle = 255; dutyCycle >= 0; dutyCycle--)
      {
        ledcWrite(0, dutyCycle);
        Serial.println(dutyCycle);
        delay(5);
      }
    }
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);
    esp_sleep_enable_timer_wakeup(30 * 60 * 1000000);
    esp_deep_sleep_start();
  }
}

void initOledDisplay()
{
  manager.createTask();
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.setTextColor(SSD1306_WHITE);
  manager.oledDisplay();

  centerText("Oled Initialized", SCREEN_HEIGHT / 2);
  centerText((resetReasonToString(esp_reset_reason())), 10);

  manager.oledDisplay();
  Serial.println("OLed display initialized");
}

void initLedDisplay()
{
  LedDisplay.setBrightness(0);
  LedDisplay.showNumberDecEx(8888, 0b11100000);
  Serial.println("Led display initialized");
}

void initLightSensor()
{
  lightMeter.begin();
  lightMeter.setActiveMode();
  lightMeter.setGain(0x04);
  lightMeter.configureMeasurement(0x00, 0x02);
}

void initButtons()
{
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_CONFIRM_PIN, INPUT_PULLUP);
  pinMode(BUTTON_EXIT_PIN, INPUT_PULLUP);
  Serial.println("Buttons initialized");
}

int melody[] = {
    NOTE_C7, NOTE_G7, NOTE_C8};

int noteDurations[] = {
    16, 11, 16};

void initBuzzer()
{
  pinMode(BUZZER_PIN, OUTPUT);
  ledcSetup(0, 2000, 8);
  ledcAttachPin(BUZZER_PIN, 0);
#ifdef START_SOUND
  Serial.println("Buzzer initialized");
  for (int i = 0; i < sizeof(melody) / sizeof(melody[0]); i++)
  {
    int noteDuration = 1000 / noteDurations[i];
    tone(BUZZER_PIN, melody[i], noteDuration);

    noTone(BUZZER_PIN);
  }
#endif
}

void initTempSensor()
{
   if (! sht4.begin()) {
    Serial.println("Couldn't find SHT4x");
    while (1) delay(1);
  }
  Serial.println("Found SHT4x sensor");
  Serial.print("Serial number 0x");
  Serial.println(sht4.readSerial(), HEX);

  // You can have 3 different precisions, higher precision takes longer
  sht4.setPrecision(SHT4X_HIGH_PRECISION);
  switch (sht4.getPrecision()) {
     case SHT4X_HIGH_PRECISION: 
       Serial.println("High precision");
       break;
     case SHT4X_MED_PRECISION: 
       Serial.println("Med precision");
       break;
     case SHT4X_LOW_PRECISION: 
       Serial.println("Low precision");
       break;
  }

  // You can have 6 different heater settings
  // higher heat and longer times uses more power
  // and reads will take longer too!
  sht4.setHeater(SHT4X_NO_HEATER);
  switch (sht4.getHeater()) {
     case SHT4X_NO_HEATER: 
       Serial.println("No heater");
       break;
     case SHT4X_HIGH_HEATER_1S: 
       Serial.println("High heat for 1 second");
       break;
     case SHT4X_HIGH_HEATER_100MS: 
       Serial.println("High heat for 0.1 second");
       break;
     case SHT4X_MED_HEATER_1S: 
       Serial.println("Medium heat for 1 second");
       break;
     case SHT4X_MED_HEATER_100MS: 
       Serial.println("Medium heat for 0.1 second");
       break;
     case SHT4X_LOW_HEATER_1S: 
       Serial.println("Low heat for 1 second");
       break;
     case SHT4X_LOW_HEATER_100MS: 
       Serial.println("Low heat for 0.1 second");
       break;
  }
}