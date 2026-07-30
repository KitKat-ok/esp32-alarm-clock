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

    apds.setProxGain(APDS9960_PGAIN_4X);

    apds.setProxPulse(APDS9960_PPULSELEN_16US, 8);

    apds.setProximityInterruptThreshold(0, PROXIMITY_THRESHOLD, 4);
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

uint16_t readColorTemperature(const ColorData &data)
{
    return apds.calculateColorTemperature(data.red, data.green, data.blue);
}

uint16_t calculateLuxWithIRCompensation(const ColorData &data)
{
    // 1. Estimate IR component
    float ir = (float)(data.red + data.green + data.blue - data.ambient) / 2.0f;
    if (ir < 0.0f) ir = 0.0f;

    // 2. Subtract IR from Clear to get true visible photopic intensity
    float visibleClear = (float)data.ambient - ir;
    if (visibleClear < 0.0f) visibleClear = 0.0f;

    // 3. Convert visible light count to Lux
    float lux = visibleClear * 0.46f;

    return (uint16_t)lux;
}