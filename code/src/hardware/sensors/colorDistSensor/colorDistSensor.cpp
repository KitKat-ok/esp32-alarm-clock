#include "colorDistSensor.h"
#include <Wire.h>

Adafruit_APDS9960 apds = Adafruit_APDS9960();
bool colorSensorInitialized = false;

uint8_t proxBaseline = 0;
constexpr uint8_t PROX_SATURATION_VALUE = 255; 

void calibrateProximity()
{
    if (!colorSensorInitialized) return;

    uint32_t sum = 0;
    uint8_t validSamples = 0;

    if (lockI2C())
    {
        // Flush the first few readings as they can be unstable right after boot
        for(uint8_t i = 0; i < 5; i++) {
            apds.readProximity();
            delay(5);
        }

        // Take 50 clean samples
        for (uint8_t i = 0; i < 50; i++)
        {
            sum += apds.readProximity();
            delay(5); 
            validSamples++;
        }
        unlockI2C();
    }

    if (validSamples > 0) {
        proxBaseline = sum / validSamples;
        Serial.print(F("[ColorSensor] Proximity baseline calibrated to: "));
        Serial.println(proxBaseline);
    }
}

float readProximityDistance()
{
    if (!colorSensorInitialized) return -1.0f;

    uint8_t raw = 0;

    if (lockI2C())
    {
        raw = apds.readProximity();
        unlockI2C();
    }

    // Handle ADC saturation (object is physically touching or highly reflective)
    if (raw == PROX_SATURATION_VALUE) return 1.0f; 

    // Nothing detected or below baseline noise
    // We add a small buffer (+2) to the baseline to prevent jittery false-positives
    if (raw <= (proxBaseline + 2)) return -1.0f;

    float corrected = (float)(raw - proxBaseline);

    /*
        Optical Distance Approximation
        Note: This will heavily depend on the reflectivity of the object.
        Adjust the '300.0f' constant for your specific physical enclosure.
    */
    float distance = 300.0f / corrected;

    // Clamp values
    if (distance < 1.0f) distance = 1.0f;
    if (distance > 50.0f) distance = 50.0f;

    return distance;
}

#if GESTURES_ENABLED == true
void setGestureInterrupt(bool enable)
{
    if (!colorSensorInitialized) return;

    if (lockI2C())
    {
        apds.setGestureIntEnable(enable);
        unlockI2C();
    }
}
#endif

void initColorSensor()
{
    bool initialized = false;

    if (lockI2C())
    {
        if (apds.begin())
        {
            initialized = true;

            // Enable core engines
            apds.enableColor(true);
            apds.enableProximity(true);

            /* --- FIX: COLOR & LUX ACCURACY ---
               Boost the integration time and gain so the sensor can actually "see" 
               enough light to do accurate math. 
               ATIME: 219 equates to ~100ms integration time (default is usually ~2ms).
               AGAIN: 16x gain boosts the signal for indoor lighting.
            */
            apds.setADCIntegrationTime(60); 
            apds.setADCGain(APDS9960_AGAIN_4X);

#if GESTURES_ENABLED == true
            apds.enableGesture(true);
            apds.setGestureDimensions(APDS9960_DIMENSIONS_LEFT_RIGHT);
            apds.setGestureGain(APDS9960_GGAIN_2);
            apds.setGestureProximityThreshold(15);
            apds.setGestureFIFOThreshold(APDS9960_GFIFO_4);
#else
            apds.setProximityInterruptThreshold(0, PROXIMITY_THRESHOLD, 4);
            apds.enableProximityInterrupt();
#endif
            
            // --- FIX: PROXIMITY STRENGTH ---
            // 4x gain and 8 pulses are the datasheet recommendations for standard use.
            apds.setProxGain(APDS9960_PGAIN_4X);
            apds.setProxPulse(APDS9960_PPULSELEN_16US, 8);
        }
        unlockI2C();
    }

    if (!initialized)
    {
        Serial.println(F("[ColorSensor] Initialization failed! Check wiring."));
        colorSensorInitialized = false;
        return;
    }

    colorSensorInitialized = true;

#if GESTURES_ENABLED == true
    setGestureInterrupt(true);
#endif

    Serial.println(F("[ColorSensor] Initialized successfully."));
    
    // Calibrate empty air
    calibrateProximity();
}

bool readColorData(ColorData &data)
{
    if (!colorSensorInitialized) return false;

    bool success = false;

    if (lockI2C())
    {
        if (apds.colorDataReady())
        {
            // Remember: These populate with 16-bit values (0 to 65535)
            apds.getColorData(&data.red, &data.green, &data.blue, &data.ambient);
            success = true;
        }
        unlockI2C();
    }

    return success;
}

uint16_t readAmbientLight()
{
    if (!colorSensorInitialized) return 0;

    uint16_t c = 0;

    if (lockI2C())
    {
        if (apds.colorDataReady())
        {
            uint16_t dummy = 0;
            apds.getColorData(&dummy, &dummy, &dummy, &c);
        }
        unlockI2C();
    }

    return c;
}

uint16_t readColorTemperature(const ColorData &data)
{
    // Now that integration time and gain are boosted, this math will work correctly.
    return apds.calculateColorTemperature(data.red, data.green, data.blue);
}

uint16_t calculateLux(const ColorData &data)
{
    // The library uses a physics-based matrix specifically tuned for the 
    // APDS-9960's internal IR and UV filters. 
    return apds.calculateLux(data.red, data.green, data.blue);
}

void disableColorSensor()
{
    if (!colorSensorInitialized) return;

    if (lockI2C())
    {
        // 1. Disable active feature engines
        apds.enableColor(false);
        apds.enableProximity(false);

#if GESTURES_ENABLED == true
        apds.enableGesture(false);
        apds.setGestureIntEnable(false);
#else
        apds.disableProximityInterrupt();
#endif

        // 2. Shut off power to the APDS-9960 internal engine
        apds.enable(false);

        unlockI2C();
    }

    Serial.println(F("[ColorSensor] Entered low-power sleep mode."));
}

void enableColorSensor()
{
    if (!colorSensorInitialized) return;

    if (lockI2C())
    {
        // 1. Re-enable main device power
        apds.enable(true);

        // 2. Re-enable required feature engines
        apds.enableColor(true);
        apds.enableProximity(true);

#if GESTURES_ENABLED == true
        apds.enableGesture(true);
        apds.setGestureIntEnable(true);
#else
        apds.enableProximityInterrupt();
#endif

        unlockI2C();
    }

    // Short stabilization delay after waking up the hardware
    delay(10);

    Serial.println(F("[ColorSensor] Woke up and restored normal mode."));
}