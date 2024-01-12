#include "hardware.h"

OLED_SSD1306_Chart display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
TM1637Display LedDisplay = TM1637Display(CLK, DIO);
BH1750 lightMeter;

void initOledDisplay();
void initLedDisplay();
void initLightSensor();
void initBuzzer();
void initButtons();

void initHardware()
{
  delay(2000);
  Serial.begin(9600);
  initBuzzer();
  initButtons();
  initOledDisplay();
  initLedDisplay();
  initLightSensor();
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  adcAttachPin(TEMP_SENS_PIN);
  setTime(0, 0, 0, 1, 1, 1970);
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
  lightMeter.begin();
  lightMeter.configure(BH1750::CONTINUOUS_HIGH_RES_MODE);
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

void initBuzzer()
{
  pinMode(BUZZER_PIN, OUTPUT);
  ledcSetup(0, 2000, 8);
  ledcAttachPin(BUZZER_PIN, 0);
  Serial.println("Buzzer initialized");
  for (int dutyCycle = 0; dutyCycle <= 255; dutyCycle++)
  {
    ledcWrite(0, dutyCycle);
    Serial.println(dutyCycle);
  }
  for (int dutyCycle = 255; dutyCycle >= 0; dutyCycle--)
  {
    ledcWrite(0, dutyCycle);
    Serial.println(dutyCycle);
  }
}