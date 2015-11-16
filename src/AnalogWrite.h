/*
  AnalogWrite.h - Firmata library
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

  Last updated by Jeff Hoefs: November 15th, 2015
*/

#ifndef AnalogWrite_h
#define AnalogWrite_h

#include <ConfigurableFirmata.h>

#if defined AnalogOutputFirmata_h || defined ServoFirmata_h

void analogWriteCallback(byte pin, int value)
{
  if (pin < TOTAL_PINS) {
    switch (Firmata.getPinMode(pin)) {
#ifdef ServoFirmata_h
      case PIN_MODE_SERVO:
        if (IS_PIN_SERVO(pin)) {
          servoAnalogWrite(pin, value);
          Firmata.setPinState(pin, value);
        }
        break;
#endif
#ifdef AnalogOutputFirmata_h
      case PIN_MODE_PWM:
        if (IS_PIN_PWM(pin)) {
          analogWrite(PIN_TO_PWM(pin), value);
          Firmata.setPinState(pin, value);
        }
        break;
#endif
    }
  }
}

#endif
#endif
