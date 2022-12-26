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

  Last updated December 23rd, 2016
*/

#include <ConfigurableFirmata.h>
#include "AnalogOutputFirmata.h"

#ifndef ESP32

AnalogOutputFirmata::AnalogOutputFirmata()
{
}

void AnalogOutputFirmata::reset()
{
}


void AnalogOutputFirmata::setupPwmPin(byte pin)
{
    pinMode(PIN_TO_PWM(pin), OUTPUT);
    analogWrite(PIN_TO_PWM(pin), 0);
}

boolean AnalogOutputFirmata::handlePinMode(byte pin, int mode)
{
    if (mode == PIN_MODE_PWM && IS_PIN_PWM(pin)) {
        setupPwmPin(pin);
        return true;
    }
    return false;
}

void AnalogOutputFirmata::handleCapability(byte pin)
{
  if (IS_PIN_PWM(pin)) {
    Firmata.write(PIN_MODE_PWM);
    Firmata.write(DEFAULT_PWM_RESOLUTION);
  }
}

void AnalogOutputFirmata::analogWriteInternal(byte pin, uint32_t value)
{
    analogWrite(pin, (int)value);
}


#endif /* NOT ESP32 */

