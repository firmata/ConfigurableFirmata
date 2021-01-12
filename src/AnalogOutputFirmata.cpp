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
#include "AnalogFirmata.h"
#include "AnalogOutputFirmata.h"

AnalogOutputFirmata::AnalogOutputFirmata()
{
  Firmata.attach(ANALOG_MESSAGE, analogWriteCallback);
}

void AnalogOutputFirmata::reset()
{

}

#ifdef ESP32

#define LEDC_BASE_FREQ 5000
// Arduino like analogWrite
// value has to be between 0 and valueMax
void analogWrite(uint8_t channel, uint32_t value, uint32_t valueMax) {
  // calculate duty, 8191 from 2 ^ 13 - 1
  uint32_t duty = (8191 / valueMax) * min(value, valueMax);

  // write duty to LEDC
  ledcWrite(channel, duty);
}

void setupPwmPin(byte pin) {
  // Setup timer and attach timer to a led pin
  ledcSetup(pin, LEDC_BASE_FREQ, 13);
  ledcAttachPin(PIN_TO_PWM(pin), pin);
  analogWrite(PIN_TO_PWM(pin), 0);
}
#else
void setupPwmPin(byte pin)
{
    pinMode(PIN_TO_PWM(pin), OUTPUT);
    analogWrite(PIN_TO_PWM(pin), 0);
}
#endif

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

boolean AnalogOutputFirmata::handleSysex(byte command, byte argc, byte* argv)
{
  if (command == EXTENDED_ANALOG) {
    if (argc > 1) {
      int val = argv[1];
      if (argc > 2) val |= (argv[2] << 7);
      if (argc > 3) val |= (argv[3] << 14);
      analogWriteCallback(argv[0], val);
      return true;
    }
    return false;
  } else {
    return handleAnalogFirmataSysex(command, argc, argv);
  }
}
