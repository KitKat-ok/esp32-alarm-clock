#include "hardware.h"
#include "rtcMem/rtcMem.h"

AS1115 LedDisplay = AS1115(0x00);

AT42QT2120 touch_sensor(Wire, TOUCH_INTERRUPT);

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
  // waitForSerialInput();
  Serial.println("Initializing Hardware");

  initI2C();
  oledMana.initDisplay();
  rM.gpioExpander.simplerInit(true);
  initButtons();
  delay(500);
  initTouch();
  initBuzzer();
  initLedDisplay();
  initLightSensor();
  initPressureSensor();
  initTempSensor();
  initColorSensor();
  turnOnGesture();

  syncTimeLibWithRTC();
  mountLittlefs();
  setCpuFrequencyMhz(80); // stable 160,80,240 needs to be 80 for wifi
  esp_pm_config_t pm_config = {
      .max_freq_mhz = 80,
      .min_freq_mhz = 10,
      .light_sleep_enable = true,
  };
  esp_pm_configure(&pm_config);
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
return false;
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
  Serial.println("starting init of touch");

  pinMode(TOUCH_INTERRUPT, INPUT);

  touch_sensor.begin();
  touch_sensor.reset();
  delay(500);

  Serial.println("touch ic initialized");

  AT42QT2120::KeyControl kc;

  kc.setTouchEnabled(false); // replaces enable_key_output = 1 (best guess mapping)
  kc.setOutputHigh(false);   // key_output = 0
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

  touch_sensor.triggerCalibration();
  Serial.println("calibrating touch");
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

#define CMD_CANCEL  1
#define CMD_RESTART 2

TaskHandle_t ledDelayTaskHandle = NULL;

void ledAutoOffTask(void *pvParameters)
{
    while (true)
    {
        uint32_t notificationValue = 0;
        
        // Wait for configured delay OR until a command notification arrives
        BaseType_t notified = xTaskNotifyWait(
            0,
            ULONG_MAX,
            &notificationValue,
            pdMS_TO_TICKS(AUTO_OFF_DELAY_MS)
        );

        if (notified == pdTRUE)
        {
            if (notificationValue == CMD_CANCEL)
            {
                break; // Exit task early
            }
            else if (notificationValue == CMD_RESTART)
            {
                continue; // Restart delay loop
            }
        }
        else
        {
            // Delay expired after 3 minutes
            turnOffLeds();
            break;
        }
    }

    // Clean exit from within task
    ledDelayTaskHandle = NULL;
    vTaskDelete(NULL);
}

void toggleLeds(bool maxPower)
{
    if (ledsOn == true)
    {
        if (ledDelayTaskHandle != NULL)
        {
            xTaskNotify(ledDelayTaskHandle, CMD_CANCEL, eSetValueWithOverwrite);
        }
        turnOffLeds();
    }
    else
    {
        turnOnLeds(maxPower);

        if (ledDelayTaskHandle != NULL)
        {
            xTaskNotify(ledDelayTaskHandle, CMD_RESTART, eSetValueWithOverwrite);
        }
        else
        {
            xTaskCreate(
                ledAutoOffTask,
                "LED_Off_Task",
                2048,
                NULL,
                1,
                &ledDelayTaskHandle
            );
        }
    }
}

void initButtons()
{
  turnOnButtons();
  Serial.println("Buttons initialized");
}

int melody[] = {
    NOTE_DS8, NOTE_G7, NOTE_C8};

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