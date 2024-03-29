/*
  I2CFirmata.h - Firmata library
  Copyright (C) 2006-2008 Hans-Christoph Steiner.  All rights reserved.
  Copyright (C) 2010-2011 Paul Stoffregen.  All rights reserved.
  Copyright (C) 2009 Shigeru Kobayashi.  All rights reserved.
  Copyright (C) 2013 Norbert Truchsess. All rights reserved.
  Copyright (C) 2009-2016 Jeff Hoefs.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.

  I2CFirmata.cpp has been merged into this header file as a hack to avoid having to
  include Wire.h for every arduino sketch that includes ConfigurableFirmata.

  Last updated by Jeff Hoefs: January 23rd, 2015
*/

#ifndef I2CFirmata_h
#define I2CFirmata_h

#include <ConfigurableFirmata.h>
#include "FirmataFeature.h"
#include "FirmataReporting.h"

#define I2C_WRITE                   0B00000000
#define I2C_READ                    0B00001000
#define I2C_READ_CONTINUOUSLY       0B00010000
#define I2C_STOP_READING            0B00011000
#define I2C_READ_WRITE_MODE_MASK    0B00011000
#define I2C_10BIT_ADDRESS_MODE_MASK 0B00100000
#define I2C_END_TX_MASK             0B01000000
#define I2C_10BIT_ADDRESS_MASK      0B00000111
#define I2C_STOP_TX                 1
#define I2C_RESTART_TX              0
#define I2C_MAX_QUERIES             8
#define I2C_REGISTER_NOT_SPECIFIED  -1

/* i2c data */
struct i2c_device_info {
  byte addr;
  int reg;
  byte bytes;
  byte stopTX;
};

class I2CFirmata: public FirmataFeature
{
  public:
    I2CFirmata();
    boolean handlePinMode(byte pin, int mode);
    void handleCapability(byte pin);
    boolean handleSysex(byte command, byte argc, byte* argv);
    void reset();
    void report(bool elapsed) override;

  private:
    /* for i2c read continuous more */
    i2c_device_info query[I2C_MAX_QUERIES];

    byte i2cRxData[32];
    boolean isI2CEnabled;
    signed char queryIndex;
    unsigned int i2cReadDelayTime;  // default delay time between i2c read request and Wire.requestFrom()

    void readAndReportData(byte address, int theRegister, byte numBytes, byte stopTX, byte seqenceNo);
    void handleI2CRequest(byte argc, byte *argv);
    boolean handleI2CConfig(byte argc, byte *argv);
    boolean enableI2CPins();
    void disableI2CPins();
};



#endif /* I2CFirmata_h */
