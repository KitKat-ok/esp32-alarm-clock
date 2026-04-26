#include "hardware.h"
#include "rtcMem/rtcMem.h"

AS1115 LedDisplay = AS1115(0x00);

AT42QT2120 touch_sensor(Wire, TOUCH_INTERRUPT);

void initOledDisplay();
void initLedDisplay();
void initBuzzer();
void initButtons();
void initTouch();
void mountLittlefs();

void waitForSerialInput()
{
  const int theDelay = 500;
  Serial.flush();

  while (true)
  {
    delay(theDelay / 2);
    if (Serial.available() > 0)
    {
      String input = Serial.readString();
      if (input.indexOf("123") >= 0)
      {
        Serial.println("Received input! launching in 3..");
        delay(theDelay);
        Serial.println("2...");
        delay(theDelay);
        Serial.println("1...");
        delay(theDelay);
        Serial.println("Go!");
        break;
      }
    }
  }
}

void initHardware()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  waitForSerialInput();
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
  rM.gpioExpander.simplerInit(true);
  initButtons();
  initTouch();
  initBuzzer();
  initOledDisplay();
  initLedDisplay();
  initLightSensor();
  initPressureSensor();
  initTempSensor();
  syncTimeLibWithRTC();
  mountLittlefs();
  Serial.println("Hardware initialized");
}

void mountLittlefs()
{
  if (!LittleFS.begin(false, "/littlefs", 65, "littlefs"))
  {
    Serial.println("LittleFS mount failed, formatting...");
    if (!LittleFS.format())
    {
      Serial.println("LittleFS format failed");
    }
    if (!LittleFS.begin(false, "/littlefs", 65, "littlefs"))
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
  // LedDisplay.writeRegister(DIG01_INTENSITY, 0x0F);
  // LedDisplay.writeRegister(DIG23_INTENSITY, 0x00);
  // LedDisplay.writeRegister(DIG45_INTENSITY, 0x0F);
  // LedDisplay.writeRegister(DIG67_INTENSITY, 0x00);
  Serial.println("Led display initialized");
}

void initTouch()
{
  pinMode(TOUCH_INTERRUPT, INPUT);

  touch_sensor.begin();
  touch_sensor.reset();
  delay(500);

  AT42QT2120::KeyControl kc;

  kc.setTouchEnabled(false); // replaces enable_key_output = 1 (best guess mapping)
  kc.setOutputHigh(false);  // key_output = 0
  kc.setAdjacentKeySuppressionGroup(1);
  kc.setGuardEnabled(false); // guard = 0

  for (uint8_t i = 0; i < 12; i++)
  {
    touch_sensor.setKeyControl(i, kc);
  }

  kc.setTouchEnabled(true); // replaces enable_key_output = 0

  for (uint8_t i = 0; i <= 6; i++)
  {
    touch_sensor.setKeyControl(i, kc);
  }

  touch_sensor.setKeyDetectThreshold(0, 12);
  touch_sensor.setKeyDetectThreshold(1, 12);
  touch_sensor.setKeyDetectThreshold(2, 12);
  touch_sensor.setKeyDetectThreshold(3, 12);
  touch_sensor.setKeyDetectThreshold(4, 12);
  touch_sensor.setKeyDetectThreshold(5, 12);
  touch_sensor.setKeyDetectThreshold(6, 12);

  AT42QT2120::KeyPulseScale PulseScale;
  PulseScale.pulse = 1;
  PulseScale.scale = 2;

  for (uint8_t i = 0; i <= 6; i++)
  {
    touch_sensor.setKeyPulseScale(i, PulseScale);
  }

  touch_sensor.setDetectionIntegrator(1);

  touch_sensor.setChargeDuration(6);
  touch_sensor.setMeasurementIntervalCount(1);

  touch_sensor.setAwayDriftCompensationDuration(0);
  touch_sensor.setDriftCompensationHoldDuration(120);
  touch_sensor.setRecalibrationDelay(255);

  touch_sensor.enableSlider();

  touch_sensor.triggerCalibration();
  while (touch_sensor.calibrating())
    delay(20);

  turnOnTouch();
}

bool ledsOn = false;

void turnOnLeds(bool maxPower)
{
  ledsOn = true;
  rM.gpioExpander.setPinState(MCP_LED1_P1, false);
  rM.gpioExpander.setPinState(MCP_LED2_P1, false);

  if (maxPower == true)
  {
    rM.gpioExpander.setPinState(MCP_LED2_P2, false);
    rM.gpioExpander.setPinState(MCP_LED1_P2, false);
  }
}

void turnOffLeds()
{
  ledsOn = false;
  rM.gpioExpander.setPinState(MCP_LED1_P1, true);
  rM.gpioExpander.setPinState(MCP_LED1_P2, true);
  rM.gpioExpander.setPinState(MCP_LED2_P1, true);
  rM.gpioExpander.setPinState(MCP_LED2_P2, true);
}

void toggleLeds(bool maxPower)
{
  if (ledsOn == true)
  {
    turnOffLeds();
  }
  else
  {
    turnOnLeds(maxPower);
  }
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