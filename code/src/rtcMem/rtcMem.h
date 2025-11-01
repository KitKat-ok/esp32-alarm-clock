#pragma once

#include "../defines.h"

struct rtcMem
{
    mcp23018 gpioExpander;
};

extern rtcMem rM;
