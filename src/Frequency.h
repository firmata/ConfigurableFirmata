/*
  DigitalInputFirmata.h - Firmata library
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

#ifndef Frquency_h
#define Frequency_h

#include <ConfigurableFirmata.h>
#include "FirmataFeature.h"

#define INTERRUPT_MODE_DISABLE 0
#define INTERRUPT_MODE_LOW 1
#define INTERRUPT_MODE_HIGH 2
#define INTERRUPT_MODE_RISING 3
#define INTERRUPT_MODE_FALLING 4
#define INTERRUPT_MODE_CHANGE 5

#define FREQUENCY_SUBCOMMAND_CLEAR 0
#define FREQUENCY_SUBCOMMAND_QUERY 1
#define FREQUENCY_SUBCOMMAND_REPORT 2
#define FREQUENCY_SUBCOMMAND_FILTER 3

// This class tries to accurately measure the number of ticks per time on a specific pin.
// All pins that have interrupt capability can be used, but only one at a time. 
class Frequency: public FirmataFeature
{
  public:
    Frequency();
    void report(bool elapsed);
    void handleCapability(byte pin);
    boolean handleSysex(byte command, byte argc, byte* argv);
    boolean handlePinMode(byte pin, int mode);
    void reset();
  private:
    static void FrequencyIsr();
	void InstanceIsr();
    void reportValue(int pin);
    int _activePin;
	int32_t _reportDelay;
	int32_t _lastReport;
	volatile int32_t _ticks;
	volatile int32_t _minTicksBetweenPulses;
	volatile int32_t _lastTickTime;
};

#endif
