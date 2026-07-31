#pragma once

#include "../../defines.h"

bool initI2C();
void deInitI2C();

// Thread-safety functions
bool lockI2C(TickType_t timeoutTicks = pdMS_TO_TICKS(500));
void unlockI2C();