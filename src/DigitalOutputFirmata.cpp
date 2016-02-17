/*
  DigitalOutputFirmata.cpp - Firmata library
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

  Last updated by Jeff Hoefs: February 16th, 2016
*/

#include <ConfigurableFirmata.h>
#include "DigitalOutputFirmata.h"

DigitalOutputFirmata *DigitalOutputFirmataInstance;

void digitalOutputWriteCallback(byte port, int value)
{
  DigitalOutputFirmataInstance->digitalWritePort(port, value);
}

/*
 * Sets the value of an individual pin. Useful if you want to set a pin value but
 * are not tracking the digital port state.
 * Can only be used on pins configured as OUTPUT.
 * Cannot be used to enable pull-ups on Digital INPUT pins.
 */
void handleSetPinValueCallback(byte pin, int value)
{
  if (pin < TOTAL_PINS && IS_PIN_DIGITAL(pin)) {
    if (Firmata.getPinMode(pin) == OUTPUT) {
      digitalWrite(pin, value);
      Firmata.setPinState(pin, value);
    }
  }
}

DigitalOutputFirmata::DigitalOutputFirmata()
{
  DigitalOutputFirmataInstance = this;
  Firmata.attach(DIGITAL_MESSAGE, digitalOutputWriteCallback);
  Firmata.attach(SET_DIGITAL_PIN_VALUE, handleSetPinValueCallback);
}

boolean DigitalOutputFirmata::handleSysex(byte command, byte argc, byte* argv)
{
  return false;
}

void DigitalOutputFirmata::reset()
{

}

void DigitalOutputFirmata::digitalWritePort(byte port, int value)
{
  byte pin, lastPin, pinValue, mask = 1, pinWriteMask = 0;

  if (port < TOTAL_PORTS) {
    // create a mask of the pins on this port that are writable.
    lastPin = port * 8 + 8;
    if (lastPin > TOTAL_PINS) lastPin = TOTAL_PINS;
    for (pin = port * 8; pin < lastPin; pin++) {
      // do not disturb non-digital pins (eg, Rx & Tx)
      if (IS_PIN_DIGITAL(pin)) {
        // do not touch pins in PWM, ANALOG, SERVO or other modes
        if (Firmata.getPinMode(pin) == OUTPUT || Firmata.getPinMode(pin) == INPUT) {
          pinValue = ((byte)value & mask) ? 1 : 0;
          if (Firmata.getPinMode(pin) == OUTPUT) {
            pinWriteMask |= mask;
          } else if (Firmata.getPinMode(pin) == INPUT && pinValue == 1 && Firmata.getPinState(pin) != 1) {
            pinMode(pin, INPUT_PULLUP);
          }
          Firmata.setPinState(pin, pinValue);
        }
      }
      mask = mask << 1;
    }
    writePort(port, (byte)value, pinWriteMask);
  }
}

boolean DigitalOutputFirmata::handlePinMode(byte pin, int mode)
{
  if (IS_PIN_DIGITAL(pin) && mode == OUTPUT && Firmata.getPinMode(pin) != PIN_MODE_IGNORE) {
    digitalWrite(PIN_TO_DIGITAL(pin), LOW); // disable PWM
    pinMode(PIN_TO_DIGITAL(pin), OUTPUT);
    return true;
  }
  return false;
}

void DigitalOutputFirmata::handleCapability(byte pin)
{
  if (IS_PIN_DIGITAL(pin)) {
    Firmata.write((byte)OUTPUT);
    Firmata.write(1);
  }
}
