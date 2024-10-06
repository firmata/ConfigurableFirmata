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
#ifdef ESP32

#define LEDC_BASE_FREQ 5000
#define MAX_PWM_CHANNELS 16


AnalogOutputFirmata::AnalogOutputFirmata()
{
}

void AnalogOutputFirmata::reset()
{
    internalReset();
}


void AnalogOutputFirmata::analogWriteInternal(uint8_t pin, uint32_t value) {
    // calculate duty, 8191 from 2 ^ 13 - 1
    uint32_t valueMax = (1 << DEFAULT_PWM_RESOLUTION) - 1;
    uint32_t duty = min(value, valueMax);
    ledcWrite(pin, duty);
}

void AnalogOutputFirmata::setupPwmPin(byte pin) {

    if (!ledcAttach(pin, LEDC_BASE_FREQ, DEFAULT_PWM_RESOLUTION))
    {
        Firmata.sendStringf(F("Warning: Pin %d could not be configured for PWM (too many channels?)"), pin);
    }
	ledcWrite(pin, 0);
}

void AnalogOutputFirmata::internalReset()
{
    for (int i = 0; i < TOTAL_PINS; i++)
    {
        if (Firmata.getPinMode(i) == PIN_MODE_PWM)
        {
            ledcDetach(i);
        }
    }
}

boolean AnalogOutputFirmata::handlePinMode(byte pin, int mode)
{
    if (mode == PIN_MODE_PWM && FIRMATA_IS_PIN_PWM(pin)) {
        setupPwmPin(pin);
        return true;
    }
   
    // Unlink the channel for this pin
    if (mode != PIN_MODE_PWM && Firmata.getPinMode(pin) == PIN_MODE_PWM)
    {
        // Firmata.sendStringf(F("Detaching pin %d"), pin);
        ledcDetach(pin);
    }
    return false;
}

void AnalogOutputFirmata::handleCapability(byte pin)
{
  if (FIRMATA_IS_PIN_PWM(pin)) {
    Firmata.write(PIN_MODE_PWM);
    Firmata.write(DEFAULT_PWM_RESOLUTION);
  }
}

#endif
