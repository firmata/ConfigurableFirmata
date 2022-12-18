/*
  AnalogFirmata.h - Firmata library
  Copyright (C) 2006-2008 Hans-Christoph Steiner.  All rights reserved.
  Copyright (C) 2010-2011 Paul Stoffregen.  All rights reserved.
  Copyright (C) 2009 Shigeru Kobayashi.  All rights reserved.
  Copyright (C) 2013 Norbert Truchsess. All rights reserved.
  Copyright (C) 2009-2015 Jeff Hoefs.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.
*/

#ifndef AnalogOutputFirmata_h
#define AnalogOutputFirmata_h

#include <ConfigurableFirmata.h>
#include "FirmataFeature.h"

class AnalogOutputFirmata: public FirmataFeature
{
    friend void analogWrite(byte pin, uint32_t value);
  public:
    AnalogOutputFirmata();
    void handleCapability(byte pin);
    boolean handlePinMode(byte pin, int mode);
    boolean handleSysex(byte command, byte argc, byte* argv);
    void reset();
  private:
      void setupPwmPin(byte pin);
#if ESP32
      int getChannelForPin(byte pin);
      void internalReset();
      void analogWriteEsp32(byte pin, uint32_t value);
      // This gives the active pin for each pwm channel. -1 if unused
    byte _pwmChannelMap[16];
#endif
};

void analogWriteCallback(byte pin, int value);
#endif
