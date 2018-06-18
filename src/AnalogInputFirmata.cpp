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

  Last updated by Jeff Hoefs: November 22nd, 2015
*/

#include <ConfigurableFirmata.h>
#include "AnalogFirmata.h"
#include "AnalogInputFirmata.h"

AnalogInputFirmata *AnalogInputFirmataInstance;

void reportAnalogInputCallback(byte analogPin, int value)
{
  AnalogInputFirmataInstance->reportAnalog(analogPin, value);
}

AnalogInputFirmata::AnalogInputFirmata()
{
  AnalogInputFirmataInstance = this;
  reset();
  Firmata.attach(REPORT_ANALOG, reportAnalogInputCallback);
}

// -----------------------------------------------------------------------------
/* sets bits in a byte array to toggle the reporting of the analog pins
 */
//void FirmataClass::setAnalogPinReporting(byte pin, byte state) {
//}
void AnalogInputFirmata::reportAnalog(byte analogPin, int value)
{
  if (analogPin < TOTAL_ANALOG_PINS) {
    if (value == 0) {
      // analogInputsToReport[ANALOG_INPUTS_BYTE_INDEX(analogPin)] &=  ~ (1 << ANALOG_INPUTS_BIT_INDEX(analogPin));
      bitClear(analogInputsToReport[ANALOG_INPUTS_BYTE_INDEX(analogPin)], ANALOG_INPUTS_BIT_INDEX(analogPin));
    } else {
      // analogInputsToReport[ANALOG_INPUTS_BYTE_INDEX(analogPin)] |= (1 << ANALOG_INPUTS_BIT_INDEX(analogPin));
      bitSet(analogInputsToReport[ANALOG_INPUTS_BYTE_INDEX(analogPin)], ANALOG_INPUTS_BIT_INDEX(analogPin));
      // prevent during system reset or all analog pin values will be reported
      // which may report noise for unconnected analog pins
      if (!Firmata.isResetting()) {
        // Send pin value immediately. This is helpful when connected via
        // ethernet, wi-fi or bluetooth so pin states can be known upon
        // reconnecting.
        Firmata.sendAnalog(analogPin, analogRead(analogPin));
      }
    }
  }
  // TODO: save status to EEPROM here, if changed
}

boolean AnalogInputFirmata::handlePinMode(byte pin, int mode)
{
  if (mode == PIN_MODE_ANALOG) {
    if (IS_PIN_ANALOG(pin)) {
      reportAnalog(PIN_TO_ANALOG(pin), 1); // turn on reporting
      if (IS_PIN_DIGITAL(pin)) {
        pinMode(PIN_TO_DIGITAL(pin), INPUT); // disable output driver
      }
    } else {
      Firmata.sendString("PIN_MODE_ANALOG set for non analog pin");
    }
    return true;
  } else if (IS_PIN_ANALOG(pin)) {
    reportAnalog(PIN_TO_ANALOG(pin), 0); // turn off reporting
  }
  return false;
}

void AnalogInputFirmata::handleCapability(byte pin)
{
  if (IS_PIN_ANALOG(pin)) {
    Firmata.write(PIN_MODE_ANALOG);
    Firmata.write(10); // 10 = 10-bit resolution
  }
}

boolean AnalogInputFirmata::handleSysex(byte command, byte argc, byte* argv)
{
  if (command == EXTENDED_ANALOG_READ) {
    if (argc > 0) {
      byte subCommand = argv[0] & 0x7F;

      if (subCommand == EXTENDED_ANALOG_READ_QUERY) {
        uint8_t analogPin = argv[1];

        if (argc > 2) {
          if (argv[2]) {
            reportAnalog(analogPin, 1);
          } else {
            reportAnalog(analogPin, 0);
            // TODO: Check if message should be sent
          }
        } else {
          Firmata.sendAnalog(analogPin, analogRead(analogPin));
        }
        return true;
      }
    }
    return false;
  }

  return handleAnalogFirmataSysex(command, argc, argv);
}

void AnalogInputFirmata::reset()
{
  // by default, do not report any analog inputs
  memset(analogInputsToReport, 0, ANALOG_INPUTS_BIT_ARRAY_SIZE);
}

void AnalogInputFirmata::report()
{
  byte pin, analogPin;
  /* ANALOGREAD - do all analogReads() at the configured sampling interval */
  for (pin = 0; pin < TOTAL_PINS; pin++) {
    if (IS_PIN_ANALOG(pin) && Firmata.getPinMode(pin) == PIN_MODE_ANALOG) {
      analogPin = PIN_TO_ANALOG(pin);
      if (bitRead(analogInputsToReport[ANALOG_INPUTS_BYTE_INDEX(analogPin)], ANALOG_INPUTS_BIT_INDEX(analogPin))) {
        Firmata.sendAnalog(analogPin, analogRead(analogPin));
      }
    }
  }
}
