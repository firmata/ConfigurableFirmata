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
    for (int i = 0; i < MAX_PWM_CHANNELS; i++)
    {
        _pwmChannelMap[i] = 255;
    }
}

void AnalogOutputFirmata::reset()
{
    internalReset();
}


void AnalogOutputFirmata::analogWriteInternal(uint8_t pin, uint32_t value) {
    // calculate duty, 8191 from 2 ^ 13 - 1
    uint32_t valueMax = (1 << DEFAULT_PWM_RESOLUTION) - 1;
    uint32_t duty = min(value, valueMax);
    // write duty to matching channel number
    int channel = getChannelForPin(pin);
    if (channel != 255)
    {
        ledcWrite(channel, duty);
        // Firmata.sendStringf(F("Channel %d has duty %d/%d"),  channel, duty, valueMax);
    }
    else
	{
		Firmata.sendStringf(F("Error: Pin %d is not set to PWM"), pin);
	}
}

int AnalogOutputFirmata::getChannelForPin(byte pin)
{
    for (int i = 0; i < MAX_PWM_CHANNELS; i++)
    {
        if (_pwmChannelMap[i] == pin)
        {
            return i;
        }
    }

    return 255;
}

void AnalogOutputFirmata::setupPwmPin(byte pin) {
  // Setup timer and attach timer to a led pin
    int channel = getChannelForPin(pin);
    if (channel == 255) // pin already assigned to a channel?
    {
        int i;
        for (i = 0; i < MAX_PWM_CHANNELS; i++)
        {
            if (_pwmChannelMap[i] == 255)
            {
                channel = i;
                break;
            }
        }

        if (i >= MAX_PWM_CHANNELS)
        {
            Firmata.sendStringf(F("Unable to setup pin %d for PWM - no more channels."), pin);
            return;
        }
        
		Firmata.sendStringf(F("Assigning channel %d to pin %d"), channel, pin);
        _pwmChannelMap[channel] = pin;
        pinMode(pin, OUTPUT);
        ledcSetup(channel, LEDC_BASE_FREQ, DEFAULT_PWM_RESOLUTION);
        ledcAttachPin(pin, channel);
        ledcWrite(pin, 0);
		return;
    }
	
	Firmata.sendStringf(F("Warning: Pin %d already assigned to channel %d"), pin, channel);
	ledcWrite(pin, 0);
}

void AnalogOutputFirmata::internalReset()
{
    for (int i = 0; i < MAX_PWM_CHANNELS; i++)
    {
        if (_pwmChannelMap[i] != 255)
        {
            ledcDetachPin(_pwmChannelMap[i]);
        }
        _pwmChannelMap[i] = 255;
    }
}

boolean AnalogOutputFirmata::handlePinMode(byte pin, int mode)
{
    if (mode == PIN_MODE_PWM && IS_PIN_PWM(pin)) {
        setupPwmPin(pin);
        return true;
    }
    int channel = 255;
    // Unlink the channel for this pin
    if (mode != PIN_MODE_PWM && (channel = getChannelForPin(pin)) != 255)
    {
		Firmata.sendStringf(F("Detaching pin %d"), pin);
        ledcDetachPin(pin);
        _pwmChannelMap[channel] = 255;
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

#endif
