#include "hardware.h"
#include "rtcMem/rtcMem.h"

AS1115 LedDisplay = AS1115(0x00);
#define AL_ADDR 0x10
SparkFun_Ambient_Light lightMeter(AL_ADDR);

Adafruit_SHT4x sht4 = Adafruit_SHT4x();

AT42QT2120 touch_sensor(Wire, TOUCH_INTERRUPT);

void initOledDisplay();
void initLedDisplay();
void initLightSensor();
void initBuzzer();
void initButtons();
void initTempSensor();
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

void initLightSensor()
{
  // Possible values: .125, .25, 1, 2
  // Both .125 and .25 should be used in most cases except darker rooms.
  // A gain of 2 should only be used if the sensor will be covered by a dark
  // glass.
  float gain = 1;

  // Possible integration times in milliseconds: 800, 400, 200, 100, 50, 25
  // Higher times give higher resolutions and should be used in darker light.
  int time = 400;

  if (lightMeter.begin(Wire))
    Serial.println("Ready to sense some light!");
  else
    Serial.println("Could not communicate with the sensor!");

  // Again the gain and integration times determine the resolution of the lux
  // value, and give different ranges of possible light readings. Check out
  // hoookup guide for more info.
  lightMeter.setGain(gain);
  lightMeter.setIntegTime(time);

  Serial.println("Reading settings...");
  Serial.print("Gain: ");
  float gainVal = lightMeter.readGain();
  Serial.print(gainVal, 3);
  Serial.print(" Integration Time: ");
  int timeVal = lightMeter.readIntegTime();
  Serial.println(timeVal);
}

void initTouch()
{
  pinMode(TOUCH_INTERRUPT, INPUT);

  touch_sensor.begin();
  touch_sensor.reset();
  delay(500);

  AT42QT2120::KeyControl kc;
  kc.enable_key_output = 1;
  kc.key_output = 0;
  kc.adjacent_key_suppression_group = 1;
  kc.guard = 0;

  for (uint8_t i = 0; i < 12; i++)
  {
    touch_sensor.setKeyControl(i, kc);
  }

  kc.key_output = 1;
  kc.enable_key_output = 0;
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

  touch_sensor.setAwayDriftCompensationDuration(50);
  touch_sensor.setDriftCompensationHoldDuration(120);
  touch_sensor.setRecalibrationDelay(80);

  touch_sensor.enableSlider();

  touch_sensor.triggerCalibration();
  while (touch_sensor.calibrating())
    delay(20);

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
  else
  {
    Serial.println("Found SHT4x sensor");
  }
  sht4.setPrecision(SHT4X_HIGH_PRECISION);
  sht4.setHeater(SHT4X_NO_HEATER);
}