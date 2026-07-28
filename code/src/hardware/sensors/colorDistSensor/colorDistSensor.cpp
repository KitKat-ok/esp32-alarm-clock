#include "colorDistSensor.h"

Adafruit_APDS9960 apds = Adafruit_APDS9960();

#if GESTURES_ENABLED == true
void setGestureInterrupt(bool enable)
{
    Wire.beginTransmission(APDS9960_ADDRESS);
    Wire.write(0xAB); // GCONF4 Register
    Wire.endTransmission(false);

    Wire.requestFrom((uint8_t)APDS9960_ADDRESS, (uint8_t)1);
    if (Wire.available())
    {
        uint8_t reg = Wire.read();

        if (enable)
        {
            reg |= 0x02; // Set bit 1 (GIEN)
        }
        else
        {
            reg &= ~0x02; // Clear bit 1 (GIEN)
        }

        Wire.beginTransmission(APDS9960_ADDRESS);
        Wire.write(0xAB);
        Wire.write(reg);
        Wire.endTransmission();
    }
}
#endif

void initColorSensor()
{
    // Adafruit's begin() sets defaults for I2C and hardware registers
    if (!apds.begin())
    {
        Serial.println(F("[ColorSensor] Initialization failed!"));
        return;
    }

    // Enable color sensor
    apds.enableColor(true);
    apds.enableProximity(true);

#if GESTURES_ENABLED == true
    // --- GESTURE CONFIGURATION ---
    apds.enableProximity(true);
    apds.enableGesture(true);

    apds.setGestureDimensions(APDS9960_DIMENSIONS_LEFT_RIGHT);
    apds.setGestureGain(APDS9960_GGAIN_2);
    apds.setGestureProximityThreshold(15);
    apds.setGestureFIFOThreshold(APDS9960_GFIFO_4);

    setGestureInterrupt(true);

#else
    // --- FULL PROXIMITY INTERRUPT CONFIGURATION ---
        Serial.println("help");

    // 1. Enable proximity hardware engine

    // 2. Set Receiver Gain (Options: APDS9960_PGAIN_1X, 2X, 4X, 8X)
    apds.setProxGain(APDS9960_PGAIN_4X);

    // 3. Set LED Pulse Length & Count (16us pulse width, 8 pulses)
    apds.setProxPulse(APDS9960_PPULSELEN_16US, 8);

    // 4. Set Hardware Interrupt Thresholds
    // Format: setProximityInterruptThreshold(low_threshold, high_threshold, persistence)
    // Low = 0 (triggers when object moves away)
    // High = 50 (triggers when object gets close enough; scale 0-255)
    // Persistence = 4 (requires 4 consecutive readings before firing interrupt to filter noise)
    apds.setProximityInterruptThreshold(PROXIMITY_THRESHOLD + 30, PROXIMITY_THRESHOLD, 4);

    // 5. Enable Proximity Interrupt on the hardware INT pin
    apds.enableProximityInterrupt();

#endif

    Serial.println(F("[ColorSensor] Initialized successfully."));
}

bool readColorData(ColorData &data)
{
    if (!apds.colorDataReady())
    {
        return false;
    }

    // Adafruit's getColorData takes pointers to uint16_t for r, g, b, c
    apds.getColorData(&data.red, &data.green, &data.blue, &data.ambient);
    return true;
}

uint16_t readAmbientLight()
{
    uint16_t r = 0, g = 0, b = 0, c = 0;

    if (!apds.colorDataReady())
    {
        return 0;
    }

    apds.getColorData(&r, &g, &b, &c);
    return c;
}