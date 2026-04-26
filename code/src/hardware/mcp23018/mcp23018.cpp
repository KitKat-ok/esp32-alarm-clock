#include "mcp23018.h"

#define EMPTY_REG 0
#define FULL_REG 0xFFFF

bool ignoreInterrupt = false;
void manageGpioExpanderInt()
{
  if (ignoreInterrupt == false)
  {
#if DEBUG
    // Serial.println("mcpD"); // Potential crash
#endif
    ignoreInterrupt = true;
    interruptedButton = Unknown;
    resumeButtonTask();
  }
}

mcp23018::mcp23018() {}

bool mcp23018::simplerInit(bool withDefault)
{
  if (inited == false && initOngoing == false)
  {
    mcpMutex.lock();
    if (inited == true)
    {
      mcpMutex.unlock();
      return true;
    }
    if (initCount > 5)
    {
      mcpMutex.unlock();
      return false;
    }
    initOngoing = true;
    if (resetVerify(withDefault) == false)
    {
      initOngoing = false;
      Serial.println("Failed to reset-verify the expander");
      initCount = initCount + 1;
      delay(10);
      mcpMutex.unlock();
      return simplerInit();
    }
    mcpMutex.unlock();
    initOngoing = false;
    inited = true;
  }
  return true;
}

void mcp23018::setDefaultInterruptsEsp()
{
  // This is not needed here?
  // if (simplerInit() == false)
  //{
  // return;
  //}
  Serial.println("Attaching gpio expander interrupt pin");
  pinMode(MCP_INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(MCP_INTERRUPT_PIN), manageGpioExpanderInt, FALLING);
}

inkButtonStates mcp23018::manageInterrupts()
{
  if (simplerInit() == false)
  {
    return Unknown;
  }
  digitalRead(MCP_INTERRUPT_PIN);
  Serial.println("Launched manageInterrupts");
  // For an unknown reason to me, it doesn't work if this is not called here:
  // Maybe it clears the interrupt or something, idk, not worth my time

  // First read the cause
  uint16_t gpio_cause = readRegister(INTF);
  // Then we read the interrupts
  uint16_t gpio_ints = readRegister(INTCAP);
  mcp23018::digitalRead(MCP_5V);
  // We disable all interrupts, we don't want new ones now
  // writeRegister(GPINTEN, EMPTY_REG); // or not OR NOT we can't in fact do that
  Serial.println("Interrupt bits: " + uint16ToBinaryString(gpio_ints));
  Serial.println("Interrupt cause: " + uint16ToBinaryString(gpio_cause));
  // dumpAllRegisters();
  int interrupt_state = ::digitalRead(MCP_INTERRUPT_PIN);
  Serial.println("MCP Interrupt State " + String(interrupt_state));
  //  What is going on here
  // I want a break call here
  bool fiveVPin = checkBit(gpio_cause, MCP_5V);
  if (fiveVPin == true)
  {
    if (fiveVPin == true)
    {
      powerConnected = mcp23018::digitalRead(MCP_5V);
    }
    return None;
  }

  if (checkBit(gpio_cause, BACK_PIN) == true)
  {
    Serial.println("Gpio expander back");
    return Back;
  }

  if (checkBit(gpio_cause, MENU_PIN) == true)
  {
    Serial.println("Gpio expander menu");
    return Menu;
  }

  if (checkBit(gpio_cause, DOWN_PIN) == true)
  {
    Serial.println("Gpio expander down pin");
    return Down;
  }

  if (checkBit(gpio_cause, UP_PIN) == true)
  {
    Serial.println("Gpio expander up pin");
    return Up;
  }

  return None;
}

bool mcp23018::manageInterruptsExit()
{
  if (simplerInit() == false)
  {
    return false;
  }
  uint16_t gpio_ints = readRegister(INTCAP);
  Serial.println("Interrupt bits: " + uint16ToBinaryString(gpio_ints));
  Serial.println("Restoring interrupts");
  Serial.println("Exiting the interrupt thing");
  int interrupt_state = ::digitalRead(MCP_INTERRUPT_PIN);
  Serial.println("MCP Interrupt State " + String(interrupt_state));
  if (interrupt_state == 0)
  {
    Serial.println("Voltage still low after reading register, running again...");
    ignoreInterrupt = false;
    manageGpioExpanderInt();
    return false;
  }
  ignoreInterrupt = false;
  return true;
}

bool mcp23018::resetVerify(bool withDefault)
{
#if MCP_GPIO_EXPANDER_DISABLE == true && DEBUG == true
  return false;
#endif
  // When this fails, we are lost
  if (initI2C() == false)
  {
    return false;
  }

  // To clear if it exists...
  readRegister(INTCAP);

  // Init to default value
  writeRegister(0, FULL_REG);
  for (byte i = 2; i < 22; i = i + 2)
  {
    writeRegister(i, EMPTY_REG);
  }
  // isDebug(dumpAllRegisters());

  // Clear it again, to be sure...
  delay(10);
  readRegister(INTCAP);

  // Verify
  bool somethingWrong = false;
  if (readRegister(0) != FULL_REG)
  {
    Serial.println("First register is wrong");
    somethingWrong = true;
  }
  for (uint8_t i = 2; i < 22; i = i + 2)
  {
    // 16 is weird, it persist but the voltage is what really indicates an interrupt
    if (i == 18 || i == 16)
    {
      continue;
    }
    if (readRegister(i) != EMPTY_REG)
    {
      Serial.println("Register " + String(i) + " is wrong");
      somethingWrong = true;
    }
  }
  if (somethingWrong == true)
  {
    Serial.println("Something is really wrong with the expander!");
    dumpAllRegisters();
    return false;
  }

  // IOCON register
  // mirror 1
  // intpol to 1 if battery voltage is below 3.0V - Not really anymore, usb charger fault
  // intcc to 1
  /*
  uint8_t bitToHigh = 0;
  if (BatteryRead() > 3.0)
  {
    bitToHigh = 2;
    expectInterruptState = FALLING;
  }
  else
  {
    bitToHigh = 1;
    expectInterruptState = RISING;
  }
  */
#if DEBUG
  if (BatteryRead() < 3.0)
  {
    Serial.println("Interrupt is already low?");
  }
#endif

  uint8_t iocon = 0b01000001;
  Serial.println("Final iocon is: " + uint8ToBinaryString(iocon));

  writeSingleRegister(IOCON, iocon);
  writeSingleRegister(IOCON + 1, iocon);

// For testing pure power consumption
#if DEBUG && true == false
  Serial.println("Dumping registers after verify and iocon apply, and exiting");
  isDebug(dumpAllRegisters());
  return true;
#endif

  iodirReg = FULL_REG;
  gpintenReg = EMPTY_REG;
  gppuReg = EMPTY_REG;
  olatReg = FULL_REG;

  intconReg = EMPTY_REG;
  defvalReg = EMPTY_REG;

  // Setting to output reduces power consumption
  // But first we need not to do a short...
  // But after setting all registers to null like above...
  for (int i = 0; i < 16; i++)
  {
    setPinState(i, true);
    setPinMode(i, MCP_OUTPUT);
  }

  dumpAllRegisters();

  if (withDefault == true)
  {
    setDefaultPinStates();
  }

  return true;
}

void mcp23018::setDefaultPinStates()
{
  Serial.println("setting mcp defaults");

  // Set pins to inputs as they are outputs now
  setPinMode(MCP_5V, MCP_INPUT);

  setPinMode(BACK_PIN, MCP_INPUT);
  setPinMode(MENU_PIN, MCP_INPUT);
  setPinMode(DOWN_PIN, MCP_INPUT);
  setPinMode(UP_PIN, MCP_INPUT);

#if RGB_DIODE
  // This does the setPinState(RGB_DIODE_PIN, true);
  setRgb(IwNone);
  // setPinMode(RGB_DIODE_RED_PIN, MCP_OUTPUT);
  // setPinMode(RGB_DIODE_GREEN_PIN, MCP_OUTPUT);
  // setPinMode(RGB_DIODE_BLUE_PIN, MCP_OUTPUT);
#endif

  setPinState(MCP_CHARGER_CONTROL_PIN, false);
  setPinMode(MCP_CHARGER_CONTROL_PIN, MCP_OUTPUT);

  setPinMode(MCP_LED1_P1,MCP_OUTPUT);
  setPinMode(MCP_LED1_P2,MCP_OUTPUT);
  setPinMode(MCP_LED2_P1,MCP_OUTPUT);
  setPinMode(MCP_LED2_P2,MCP_OUTPUT);


  setDefaultInterrupts();

  dumpAllRegisters();
}

void mcp23018::deInit()
{
  if (initI2C() == false)
  {
    return;
  }
  // rM.gpioExpander.setPinState(MCP_STAT_OUT, true); // test
  // dumpAllRegisters();
}

bool mcp23018::digitalRead(uint8_t pin)
{
  if (simplerInit() == false)
  {
    return false;
  }
  // Manage YATCHY_BACK_BTN not existing here
  // Also consider using interrupts
  return checkBit(readRegister(GPIO), pin);
}

void mcp23018::setDefaultInterrupts()
{
  setInterruptCause(BACK_PIN, true, false);
  setPinPullUp(BACK_PIN, true);
  setInterrupt(BACK_PIN, true);

  setInterruptCause(MENU_PIN, true, false);
  setPinPullUp(MENU_PIN, true);
  setInterrupt(MENU_PIN, true);

  setInterruptCause(DOWN_PIN, true, false);
  setPinPullUp(DOWN_PIN, true);
  setInterrupt(DOWN_PIN, true);

  setInterruptCause(UP_PIN, true, false);
  setPinPullUp(UP_PIN, true);
  setInterrupt(UP_PIN, true);

  setInterrupt(MCP_5V, true);

  Serial.println("Setting Mcp interrupts");
}

void mcp23018::setInterrupt(uint8_t pin, bool interrupt)
{
  if (simplerInit() == false)
  {
    return;
  }
  setBit(gpintenReg, pin, interrupt);
  // Serial.println("gpintenReg: " + uint16ToBinaryString(gpintenReg));
  writeRegister(GPINTEN, gpintenReg);
}

void mcp23018::setInterruptCause(uint8_t pin, bool enableCause, bool causeState)
{
  if (simplerInit() == false)
  {
    return;
  }
  // enableCause writes the bit to INTCON
  // causeState writes the bit to DEFVAL, but it's a NOT statement because if we want the cause to be true, we need to write the opposite which is false
  setBit(intconReg, pin, enableCause);
  setBit(defvalReg, pin, !causeState);

  writeRegister(INTCON, intconReg);
  writeRegister(DEFVAL, defvalReg);
}

// Use MCP_OUTPUT and MCP_INPUT here
void mcp23018::setPinMode(uint8_t pin, bool mode)
{
  if (simplerInit() == false)
  {
    return;
  }
  // NOT is here
  setBit(iodirReg, pin, !mode);
  // Serial.println("iodirReg: " + uint16ToBinaryString(iodirReg));
  writeRegister(IODIR, iodirReg);
}

void mcp23018::setPinState(uint8_t pin, bool state)
{
  if (simplerInit() == false)
  {
    return;
  }
  setBit(olatReg, pin, state);
  // Serial.println("olatReg: " + uint16ToBinaryString(olatReg));
  writeRegister(OLAT, olatReg);
}

void mcp23018::setPinPullUp(uint8_t pin, bool pull)
{
  if (simplerInit() == false)
  {
    return;
  }
  setBit(gppuReg, pin, pull);
  // Serial.println("gppuReg: " + uint16ToBinaryString(gppuReg));
  writeRegister(GPPU, gppuReg);
}

void mcp23018::setBit(uint16_t &val, uint8_t bit, bool state)
{
  if (state)
  {
    val |= (1 << bit);
  }
  else
  {
    val &= ~(1 << bit);
  }
}

bool mcp23018::checkBit(uint16_t val, uint8_t bit)
{
  return (val & (1 << bit)) != 0;
}

void mcp23018::writeSingleRegister(uint8_t reg, uint8_t val)
{
  Wire.beginTransmission(MCP23018_ADDRESS);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

void mcp23018::writeRegister(uint8_t reg, uint16_t val)
{
  // Serial.println("Writing value: " + uint16ToBinaryString(val));
  uint8_t byte0 = val & 0xFF;
  uint8_t byte1 = (val >> 8) & 0xFF;
  // Serial.println("Bytes to write, bare: " + String(byte0) + " " + String(byte1));
  /*
  uint8_t buffer[2];
  buffer[0] = byte0;
  buffer[1] = byte1;
  Wire.beginTransmission(MCP23018_ADDRESS);
  Wire.write(reg);
  Wire.write(buffer, 2);
  Wire.endTransmission();
  */
  Wire.beginTransmission(MCP23018_ADDRESS);
  Wire.write(reg);
  Wire.write(byte0);
  Wire.write(byte1);
  Wire.endTransmission();
}

uint16_t mcp23018::readRegister(uint8_t reg)
{
  Wire.beginTransmission(MCP23018_ADDRESS);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(MCP23018_ADDRESS, 2);
  Wire.available();
  byte data1 = Wire.read();
  byte data2 = Wire.read();
#if DEBUG
  if (Wire.available())
  {
    Serial.println("There are more bytes available?");
  }
#endif

  // Serial.println("Received pure bytes: " + String(data1) + " " + String(data2));

  uint16_t combined = (static_cast<uint16_t>(data1)) | data2 << 8;
  // Serial.println("Received data: " + uint16ToBinaryString(combined) + " in register: " + String(reg));
  return combined;
}

uint8_t mcp23018::readSingleRegister(uint8_t reg)
{
  Wire.beginTransmission(MCP23018_ADDRESS);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(MCP23018_ADDRESS, 1);
  Wire.available();
  uint8_t readed = Wire.read();
  // Serial.println("Readed single byte: " + uint8ToBinaryString(readed));
  return readed;
}

String uint16ToBinaryString(uint16_t value)
{
  String binaryString = "";
  for (int i = 15; i >= 0; i--)
  {
    binaryString += (value & (1 << i)) ? '1' : '0';
  }
  return binaryString;
}

String uint8ToBinaryString(uint8_t value)
{
  String binaryString = "";
  for (int i = 7; i >= 0; i--)
  {
    binaryString += (value & (1 << i)) ? '1' : '0';
  }
  return binaryString;
}

String decimalToHexString(int decimal)
{
  char hexString[10];
  sprintf(hexString, "%X", decimal);
  String str = String(hexString);
  if (str.length() < 2)
  {
    str = "0" + str;
  }
  return "0x" + str;
}

void mcp23018::dumpAllRegisters()
{
#if true == true
  if (simplerInit() == false)
  {
    return;
  }
  for (byte i = 0; i < 22; i = i + 2)
  {
    Serial.println("Register: " + decimalToHexString(i) + " is: " + uint16ToBinaryString(readRegister(i)));
  }
#else
  for (byte i = 0; i < 22; i++)
  {
    Serial.println("Register: " + decimalToHexString(i) + " is: " + uint8ToBinaryString(readSingleRegister(i)));
  }
#endif
}