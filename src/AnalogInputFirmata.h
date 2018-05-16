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

#ifndef AnalogInputFirmata_h
#define AnalogInputFirmata_h

#include <ConfigurableFirmata.h>
#include "FirmataFeature.h"
#include "FirmataReporting.h"

extern "C" {
#include <limits.h>
}

#define ANALOG_INPUTS_BIT_ARRAY_SIZE ((TOTAL_ANALOG_PINS / CHAR_BIT) + ((TOTAL_ANALOG_PINS % CHAR_BIT) ? 1 : 0))
#define ANALOG_INPUTS_BYTE_INDEX(b)  (b / CHAR_BIT)
#define ANALOG_INPUTS_BIT_INDEX(b)   (b % CHAR_BIT)

void reportAnalogInputCallback(byte analogPin, int value);

class AnalogInputFirmata: public FirmataFeature
{
  public:
    AnalogInputFirmata();
    void reportAnalog(byte analogPin, int value);
    void handleCapability(byte pin);
    boolean handlePinMode(byte pin, int mode);
    boolean handleSysex(byte command, byte argc, byte* argv);
    void reset();
    void report();

  private:
    /* analog inputs */
    byte analogInputsToReport[ANALOG_INPUTS_BIT_ARRAY_SIZE]; // bitwise array to store pin reporting
};

#endif
