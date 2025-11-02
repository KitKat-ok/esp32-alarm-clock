#include "hardware.h"

AS1115 LedDisplay = AS1115(0x00);
LTR_F216A lightMeter;

Adafruit_SHT4x sht4 = Adafruit_SHT4x();

void initOledDisplay();
void initLedDisplay();
void initLightSensor();
void initBuzzer();
void initButtons();
void initTempSensor();
void initTouch();
void mountLittlefs();

void initHardware()
{
  Serial.begin(115200);
  Serial.println("Initializing Hardware");
  setCpuFrequencyMhz(80); // stable 160,80,240 needs to be 80 for wifi
  esp_pm_config_t pm_config = {
      .max_freq_mhz = 80,
      .min_freq_mhz = 10,
      .light_sleep_enable = true,
  };
  esp_pm_configure(&pm_config);
  initI2C();
  oledMana.initDisplay();
  initButtons();
  initTouch();
  initBuzzer();
  initOledDisplay();
  initLedDisplay();
  initLightSensor();
  initTempSensor();
  syncTimeLibWithRTC();
  mountLittlefs();
  Serial.println("Hardware initialized");
}

void mountLittlefs()
{
  if (!LittleFS.begin())
  {
    Serial.println("LittleFS mount failed, formatting...");
    if (!LittleFS.format())
    {
      Serial.println("LittleFS format failed");
    }
    if (!LittleFS.begin())
    {
      Serial.println("LittleFS mount failed after format continue and see what happens :3");
    }
  }
  else
  {
    Serial.println("LittleFS mounted successfully");
  }
}

bool readHallSwitch()
{
  int pinState = digitalRead(HALL_SWITCH); // Read the pin state
  return (pinState == LOW);                // Return true if LOW, false if HIGH
}

void initOledDisplay()
{
  oledMana.initDisplay();
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oledMana.display();

  centerText("Oled Initialized", SCREEN_HEIGHT / 2);
  centerText((resetReasonToString(esp_reset_reason())), 10);

  oledMana.display();
  oledMana.enable();
  Serial.println("OLed display initialized");
}

void initLedDisplay()
{
  LedDisplay.init(4, 6);
  LedDisplay.clear();
  Serial.println("Led display initialized");
}

void initLightSensor()
{
  lightMeter.begin();
  lightMeter.setActiveMode();
  lightMeter.setGain(0x04);
  lightMeter.configureMeasurement(0x00, 0x02);
}

void initTouch()
{
  turnOnTouch();
}

void initButtons()
{
  turnOnButtons();
  Serial.println("Buttons initialized");
}

int melody[] = {
    NOTE_C7, NOTE_G7, NOTE_C8};

int noteDurations[] = {
    16, 11, 16};

void initBuzzer()
{
  pinMode(BUZZER_PIN, OUTPUT);
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
  if (!sht4.begin())
  {
    Serial.println("Couldn't find SHT4x");
  }
  Serial.println("Found SHT4x sensor");
  Serial.print("Serial number 0x");
  Serial.println(sht4.readSerial(), HEX);

  // You can have 3 different precisions, higher precision takes longer
  sht4.setPrecision(SHT4X_HIGH_PRECISION);
  switch (sht4.getPrecision())
  {
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
  switch (sht4.getHeater())
  {
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