#include "touch.h"

AT42QT2120 touch_sensor(Wire, TOUCH_INTERRUPT);

#define TOUCH_DEBOUNCE 50
#define LONG_PRESS_MS 600

// --- Normal Power / Initialization Settings ---
#define NORMAL_MEASUREMENT_INTERVAL 1
#define NORMAL_CHARGE_DURATION      6
#define NORMAL_PULSE_COUNT          1
#define NORMAL_PULSE_SCALE          2

bool touchActivated = false;
TaskHandle_t touchTask = NULL;

touchState currentTouch = {};
std::mutex touchMut;

bool touchPending = false;

void playTouchSound()
{
    tone(BUZZER_PIN, NOTE_A4, 1000 / 24);
}

touchState useTouch()
{
    std::lock_guard<std::mutex> lock(touchMut);

    touchState out = currentTouch;

    currentTouch.touched = false;
    currentTouch.longPress = false;

    touchPending = false;

    return out;
}

touchState useAllTouch()
{
    std::lock_guard<std::mutex> lock(touchMut);
    return currentTouch;
}

void setTouch(const touchState &t)
{
    std::lock_guard<std::mutex> lock(touchMut);
    currentTouch = t;
    touchPending = true;
}

void loopTouchTask(void *parameter)
{
    touchActivated = true;

    bool wasPressed = false;
    bool soundPlayed = false;

    uint32_t pressStart = 0;
    uint32_t lastEvent = 0;
    delay(2000); // waits for the sensor to properly init

    while (true)
    {
        bool pressed = false;
        Serial.println("Detecting touch task awakened");
        if (lockI2C())
        {
            auto status = touch_sensor.getStatus();
            unlockI2C();
            Serial.println("got status touch");

            pressed = status.any_key_touched;
            uint32_t now = millis();

            if (pressed)
            {
                if (!wasPressed && (now - lastEvent > TOUCH_DEBOUNCE))
                {
                    pressStart = now;
                    lastEvent = now;
                    soundPlayed = false;
                }

                if (!soundPlayed)
                {
                    playTouchSound();
                    soundPlayed = true;
                }

                touchState t = {};
                t.touched = true;
                inputDetected = true;
                t.longPress = (now - pressStart >= LONG_PRESS_MS);

                for (uint8_t key = 0; key < KEYS_AMMOUNT; ++key)
                {
                    if (touch_sensor.touched(status, key))
                    {
                        t.butoonsPressed[key] = true;
                    }
                    else
                    {
                        t.butoonsPressed[key] = false;
                    }
                }

                setTouch(t);

                wasPressed = true;
            }
            else
            {
                wasPressed = false;
                soundPlayed = false;
            }
        }

        if (!pressed)
        {
            vTaskSuspend(NULL);
        }

        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
}

void initTouchTask()
{
    xTaskCreate(
        loopTouchTask,
        "touchTask",
        4096,
        NULL,
        12,
        &touchTask);
}

void turnOnTouch()
{
    Serial.println("turning on touch");
    if (!touchActivated)
    {
        turnOnTouchInterrupts();
        initTouchTask();
    }
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

  for (uint8_t i = 0; i <= 6; i++)
  {
    touch_sensor.setKeyDetectThreshold(i, 12);
  }

  AT42QT2120::KeyPulseScale PulseScale;
  PulseScale.pulse = NORMAL_PULSE_COUNT;
  PulseScale.scale = NORMAL_PULSE_SCALE;

  for (uint8_t i = 0; i <= 6; i++)
  {
    touch_sensor.setKeyPulseScale(i, PulseScale);
  }

  touch_sensor.setDetectionIntegrator(1);

  touch_sensor.setChargeDuration(NORMAL_CHARGE_DURATION);
  touch_sensor.setMeasurementIntervalCount(NORMAL_MEASUREMENT_INTERVAL);

  touch_sensor.setAwayDriftCompensationDuration(0);
  touch_sensor.setDriftCompensationHoldDuration(120);
  touch_sensor.setRecalibrationDelay(255);

  touch_sensor.triggerCalibration();
  Serial.println("calibrating touch");
  while (touch_sensor.calibrating())
    delay(20);

  turnOnTouch();
}

void setTouchLowPower()
{
  if (lockI2C())
  {
    // Adjusting measurement interval to 255 (~4.08s cycle) reduces scan energy by 99.6%
    // without altering reference thresholds or triggering false interrupt wakeups.
    touch_sensor.setMeasurementIntervalCount(155);
    unlockI2C();
  }
}

void setTouchNormalPower()
{
  if (lockI2C())
  {
    // Restore active scanning interval (~16ms)
    touch_sensor.setMeasurementIntervalCount(NORMAL_MEASUREMENT_INTERVAL);
    unlockI2C();
  }
}