#include "hardware.h"

OLED_SSD1306_Chart display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
TM1637Display LedDisplay = TM1637Display(CLK, DIO);
BH1750 lightMeter;

OneWire oneWire(TEMP_SENS_PIN);
DallasTemperature tempSensor(&oneWire);

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
  analogReadResolution(12);
  initButtons();
  delay(100);
  initOledDisplay();
  delay(100);
  initLedDisplay();
  delay(100);
  LowBattery();
  delay(100);
  initBuzzer();
  delay(100);
  initLightSensor();
  delay(100);
  initTempSensor();
  setTime(0, 0, 0, 1, 1, 1970);
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
    Serial.print("Battery too low! going to sleep to prevent restarting");

    pinMode(BUZZER_PIN, OUTPUT);
    ledcSetup(0, 2000, 8);
    ledcAttachPin(BUZZER_PIN, 0);

    LedDisplay.clear();
    display.clearDisplay();
    display.display();

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
    esp_sleep_enable_timer_wakeup(30 * 60 * 1000000);
    esp_deep_sleep_start();
  }
}

void initOledDisplay()
{
  if (!display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.dim(true);
  display.setTextColor(SSD1306_WHITE);

  centerText("Oled Initialized", SCREEN_HEIGHT / 2);

  display.display();
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
  lightMeter.configure(BH1750::CONTINUOUS_HIGH_RES_MODE);
  lightMeter.measurementReady(true);
  lightMeter.setMTreg(69);
  lightMeter.begin();
  Serial.println("Light sensor initialized");
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
    NOTE_C5, NOTE_G5, NOTE_C6};

int noteDurations[] = {
    5, 5, 5};

void initBuzzer()
{
  pinMode(BUZZER_PIN, OUTPUT);
  ledcSetup(0, 2000, 8);
  ledcAttachPin(BUZZER_PIN, 0);
  Serial.println("Buzzer initialized");
  for (int i = 0; i < sizeof(melody) / sizeof(melody[0]); i++)
  {
    int noteDuration = 1000 / noteDurations[i];
    tone(BUZZER_PIN, melody[i], noteDuration);

    noTone(BUZZER_PIN);
  }
}

void initTempSensor()
{
  tempSensor.begin();
}