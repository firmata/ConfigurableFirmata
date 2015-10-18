/*
  ToneFirmata.cpp - Firmata library
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

  Last updated by Jeff Hoefs: October 18th, 2015
*/

#include <ConfigurableFirmata.h>
#include "ToneFirmata.h"

boolean ToneFirmata::handlePinMode(byte pin, int mode)
{
  if (mode == MODE_TONE) {
    if (IS_PIN_DIGITAL(pin)) {
      //digitalWrite(PIN_TO_DIGITAL(pin), LOW); // disable PWM
      //pinMode(PIN_TO_DIGITAL(pin), OUTPUT);
      return true;
    }
  }
  return false;
}

void ToneFirmata::handleCapability(byte pin)
{
  if (IS_PIN_DIGITAL(pin)) {
    Firmata.write(MODE_TONE);
    Firmata.write(14); // 14 bit frequency value
  }
}

/*==============================================================================
 * SYSEX-BASED commands
 *============================================================================*/

// TO DO: make duration optional?
// if duration is option, then noTone would need to be called to end tone
boolean ToneFirmata::handleSysex(byte command, byte argc, byte *argv)
{
  byte toneCommand, pin;
  int frequency, duration;

  if (command == TONE_MESSAGE) {

    toneCommand = argv[0];
    pin = argv[1];

    if (Firmata.getPinMode(pin) != MODE_TONE) {
      if (IS_PIN_DIGITAL(pin) && Firmata.getPinMode(pin) != IGNORE) {
        Firmata.setPinMode(pin, MODE_TONE);
      }
    }

    if (toneCommand == TONE_PLAY) {
      frequency = argv[2] + (argv[3] << 7);
      // duration is currently limited to 16,383 ms
      duration = argv[4] + (argv[5] << 7);
      tone(pin, frequency, duration);
    }
    else if (toneCommand == TONE_STOP) {
      noTone(pin);
    }
    return true;

  }
  return false;
}

/*==============================================================================
 * SETUP()
 *============================================================================*/

void ToneFirmata::reset()
{
  // currently reset is called after all pins have been reset to default so
  // the following won't work until this is resolved (if necessary)
  // for (int i=0; i < TOTAL_PINS; i++) {
  //   if (Firmata.getPinMode(i) == TONE) {
  //     noTone(i);
  //   }
  // }
}
