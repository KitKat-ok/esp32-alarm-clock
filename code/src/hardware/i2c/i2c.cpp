#include "i2c.h"

bool initedI2C = false;
uint8_t i2cInitCount = 0;

bool initI2C()
{
    if (initedI2C == false)
    {
        Serial.println("Starting to init I2C line");
        if (i2cInitCount > 5)
        {
            return false;
        }
        if (Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQ * 1000) == false)
        {
            Serial.println("Failed to begin I2C");
            i2cInitCount = i2cInitCount + 1;
            delay(10);
            return initI2C();
        } else {
            Serial.println("Inited I2C line");
        }
        initedI2C = true;
    }
    return true;
}

void deInitI2C() {
    initedI2C = false;
    i2cInitCount = 10; // This turns it off forever in this session
    bool wireEnd = Wire.end();
    Serial.println("Wire end status: " + BOOL_STR(wireEnd));
}