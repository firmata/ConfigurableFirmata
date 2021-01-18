 /*
  * I2CFirmata.cpp
  * Copied here as a hack to avoid having to include Wire.h in all sketch files that
  * include ConfigurableFirmata.h
  */

#include "ConfigurableFirmata.h"
#include "Wire.h"
#include "I2CFirmata.h"

I2CFirmata::I2CFirmata()
{
  isI2CEnabled = false;
  queryIndex = -1;
  i2cReadDelayTime = 0;  // default delay time between i2c read request and Wire.requestFrom()
}

void I2CFirmata::readAndReportData(byte address, int theRegister, byte numBytes, byte stopTX) {
  // allow I2C requests that don't require a register read
  // for example, some devices using an interrupt pin to signify new data available
  // do not always require the register read so upon interrupt you call Wire.requestFrom()
  if (theRegister != I2C_REGISTER_NOT_SPECIFIED) {
    Wire.beginTransmission(address);
    Wire.write((byte)theRegister);
    Wire.endTransmission(stopTX); // default = true
    // do not set a value of 0
    if (i2cReadDelayTime > 0) {
      // delay is necessary for some devices such as WiiNunchuck
      delayMicroseconds(i2cReadDelayTime);
    }
  }
  else {
    theRegister = 0;  // fill the register with a dummy value
  }

  Wire.requestFrom(address, numBytes);  // all bytes are returned in requestFrom

  // check to be sure correct number of bytes were returned by slave
  if (numBytes < Wire.available()) {
    Firmata.sendString(F("I2C: Too many bytes received"));
  }
  else if (numBytes > Wire.available()) {
    Firmata.sendString(F("I2C: Too few bytes received"));
    numBytes = Wire.available();
  }

  i2cRxData[0] = address;
  i2cRxData[1] = theRegister;

  for (int i = 0; i < numBytes && Wire.available(); i++) {
    i2cRxData[2 + i] = Wire.read();
  }

  // send slave address, register and received bytes
  Firmata.sendSysex(SYSEX_I2C_REPLY, numBytes + 2, i2cRxData);
}

boolean I2CFirmata::handlePinMode(byte pin, int mode)
{
  if (IS_PIN_I2C(pin)) {
    if (mode == PIN_MODE_I2C) {
      // the user must call I2C_CONFIG to enable I2C for a device
      return true;
    }
    else if (isI2CEnabled) {
      // disable i2c so pins can be used for other functions
      // the following if statements should reconfigure the pins properly
      if (Firmata.getPinMode(pin) == PIN_MODE_I2C) {
        disableI2CPins();
      }
    }
  }
  return false;
}

void I2CFirmata::handleCapability(byte pin)
{
  if (IS_PIN_I2C(pin)) {
    Firmata.write(PIN_MODE_I2C);
    Firmata.write(1); // TODO: could assign a number to map to SCL or SDA
  }
}

boolean I2CFirmata::handleSysex(byte command, byte argc, byte* argv)
{
  switch (command) {
  case I2C_REQUEST:
    if (isI2CEnabled) {
      handleI2CRequest(argc, argv);
      return true;
    }
  case I2C_CONFIG:
    return handleI2CConfig(argc, argv);
  }
  return false;
}

void I2CFirmata::handleI2CRequest(byte argc, byte* argv)
{
  byte mode;
  byte stopTX;
  byte slaveAddress;
  byte data;
  int slaveRegister;
  mode = argv[1] & I2C_READ_WRITE_MODE_MASK;
  if (argv[1] & I2C_10BIT_ADDRESS_MODE_MASK) {
    Firmata.sendString(F("10-bit addressing not supported"));
    return;
  }
  else {
    slaveAddress = argv[0];
  }

  // need to invert the logic here since 0 will be default for client
  // libraries that have not updated to add support for restart tx
  if (argv[1] & I2C_END_TX_MASK) {
    stopTX = I2C_RESTART_TX;
  }
  else {
    stopTX = I2C_STOP_TX; // default
  }

  switch (mode) {
  case I2C_WRITE:
    Wire.beginTransmission(slaveAddress);
    for (byte i = 2; i < argc; i += 2) {
      data = argv[i] + (argv[i + 1] << 7);
      Wire.write(data);
    }
    Wire.endTransmission();
    delayMicroseconds(70);
    break;
  case I2C_READ:
    if (argc == 6) {
      // a slave register is specified
      slaveRegister = argv[2] + (argv[3] << 7);
      data = argv[4] + (argv[5] << 7);  // bytes to read
    }
    else {
      // a slave register is NOT specified
      slaveRegister = I2C_REGISTER_NOT_SPECIFIED;
      data = argv[2] + (argv[3] << 7);  // bytes to read
    }
    readAndReportData(slaveAddress, (int)slaveRegister, data, stopTX);
    break;
  case I2C_READ_CONTINUOUSLY:
    if ((queryIndex + 1) >= I2C_MAX_QUERIES) {
      // too many queries, just ignore
      Firmata.sendString(F("too many queries"));
      break;
    }
    if (argc == 6) {
      // a slave register is specified
      slaveRegister = argv[2] + (argv[3] << 7);
      data = argv[4] + (argv[5] << 7);  // bytes to read
    }
    else {
      // a slave register is NOT specified
      slaveRegister = (int)I2C_REGISTER_NOT_SPECIFIED;
      data = argv[2] + (argv[3] << 7);  // bytes to read
    }
    queryIndex++;
    query[queryIndex].addr = slaveAddress;
    query[queryIndex].reg = slaveRegister;
    query[queryIndex].bytes = data;
    query[queryIndex].stopTX = stopTX;
    break;
  case I2C_STOP_READING:
    byte queryIndexToSkip;
    // if read continuous mode is enabled for only 1 i2c device, disable
    // read continuous reporting for that device
    if (queryIndex <= 0) {
      queryIndex = -1;
    }
    else {
      queryIndexToSkip = 0;
      // if read continuous mode is enabled for multiple devices,
      // determine which device to stop reading and remove it's data from
      // the array, shifiting other array data to fill the space
      for (byte i = 0; i < queryIndex + 1; i++) {
        if (query[i].addr == slaveAddress) {
          queryIndexToSkip = i;
          break;
        }
      }

      for (byte i = queryIndexToSkip; i < queryIndex + 1; i++) {
        if (i < I2C_MAX_QUERIES) {
          query[i].addr = query[i + 1].addr;
          query[i].reg = query[i + 1].reg;
          query[i].bytes = query[i + 1].bytes;
          query[i].stopTX = query[i + 1].stopTX;
        }
      }
      queryIndex--;
    }
    break;
  default:
    break;
  }
}

boolean I2CFirmata::handleI2CConfig(byte argc, byte* argv)
{
  unsigned int delayTime = (argv[0] + (argv[1] << 7));

  if (delayTime > 0) {
    i2cReadDelayTime = delayTime;
  }

  if (!isI2CEnabled) {
    enableI2CPins();
  }
  return isI2CEnabled;
}

boolean I2CFirmata::enableI2CPins()
{
  byte i;
  // is there a faster way to do this? would probaby require importing
  // Arduino.h to get SCL and SDA pins
  for (i = 0; i < TOTAL_PINS; i++) {
    if (IS_PIN_I2C(i)) {
      if (Firmata.getPinMode(i) == PIN_MODE_IGNORE) {
        return false;
      }
      // mark pins as i2c so they are ignore in non i2c data requests
      Firmata.setPinMode(i, PIN_MODE_I2C);
      pinMode(i, PIN_MODE_I2C);
    }
  }

  isI2CEnabled = true;

  Wire.begin();
  return true;
}

/* disable the i2c pins so they can be used for other functions */
void I2CFirmata::disableI2CPins()
{
  isI2CEnabled = false;
  // disable read continuous mode for all devices
  queryIndex = -1;
  // uncomment the following if or when the end() method is added to Wire library
  // Wire.end();
}

void I2CFirmata::reset()
{
  if (isI2CEnabled) {
    disableI2CPins();
  }
}

void I2CFirmata::report(bool elapsed)
{
  // report i2c data for all device with read continuous mode enabled
  if (queryIndex > -1) {
    for (byte i = 0; i < queryIndex + 1; i++) {
      readAndReportData(query[i].addr, query[i].reg, query[i].bytes, query[i].stopTX);
    }
  }
}
