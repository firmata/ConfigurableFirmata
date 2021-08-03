/*
  Frequency.cpp - Firmata library
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

#include <ConfigurableFirmata.h>
#include "Frequency.h"

Frequency *FrequencyFirmataInstance;

volatile int32_t Frequency::_ticks = 0;

Frequency::Frequency()
{
  FrequencyFirmataInstance = this;
  _activePin = -1;
  _reportDelay = 0;
  _ticks = 0;
  _lastReport = millis();
}

void Frequency::FrequencyIsr()
{
	// The ISR can't be interrupted by the main routine, therefore this is thread safe
	_ticks++;
}

boolean Frequency::handleSysex(byte command, byte argc, byte* argv)
{
  if (command != FREQUENCY_COMMAND)
  {
	  return false;
  }
  if (argc >= 2)
  {
	  byte frequencyCommand = argv[0];
	  byte pin = argv[1];
	  if (frequencyCommand == FREQUENCY_SUBCOMMAND_CLEAR)
	  {
		  if (_activePin == pin || (_activePin >= 0 && pin == 0x7F))
		  {
		  	// This cannot be -1 here
			  uint8_t interrupt = (uint8_t)digitalPinToInterrupt(_activePin);
			  detachInterrupt(interrupt);
			  _activePin = -1;
		  }
	  }
  }
  if (argc >= 5) // Expected: A command byte, a pin, the mode and a packed short
  {
	  byte frequencyCommand = argv[0];
	  byte pin = argv[1];
	  int interrupt = digitalPinToInterrupt(pin);
	  byte mode = argv[2]; // see below
	  int32_t ms = (argv[4] << 7) | argv[3];
	  // Set or query
	  if (frequencyCommand == FREQUENCY_SUBCOMMAND_QUERY)
	  {
		  if (pin >= TOTAL_PINS || interrupt < 0)
		  {
			  Firmata.sendString(F("Invalid pin number for frequency command"));
			  return true;
		  }
		  
		  if (ms > 0)
		  {
			  _lastReport = millis();
			  _reportDelay = ms;
		  }
		  if (_activePin == -1)
		  {
			  // not yet enabled on this pin
			  int internalMode = LOW;
			  switch (mode)
			  {
				  case INTERRUPT_MODE_LOW:
				  internalMode = LOW;
				  break;
				  case INTERRUPT_MODE_HIGH:
				  internalMode = HIGH;
				  break;
				  case INTERRUPT_MODE_FALLING:
				  internalMode = FALLING;
				  break;
				  case INTERRUPT_MODE_RISING:
				  internalMode = RISING;
				  break;
				  case INTERRUPT_MODE_CHANGE:
				  internalMode = CHANGE;
				  break;
				  default:
				  internalMode = -1;
				  break;
			  }
			  if (internalMode >= 0)
			  {
				  attachInterrupt(digitalPinToInterrupt(pin), FrequencyIsr, internalMode);
				  _activePin = pin;
			  }
			  
			  Firmata.setPinMode(pin, PIN_MODE_FREQUENCY);
			  // Firmata.sendStringf(F("Frequency mode enabled with delay %ld on pin %ld"), 8, (int32_t)_reportDelay, (int32_t)pin);
		  }
		  else if (_activePin != pin)
		  {
			  Firmata.sendString(F("Cannot change pin number while active"));
			  return true;
		  }
		  reportValue(pin);
	  }
  }
  return true;
}

void Frequency::report(bool elapsed)
{
	int mi = millis();
	if (mi < _lastReport || mi > (_lastReport + _reportDelay))
	{
		if (_activePin >= 0)
		{
			reportValue(_activePin);
		}
		_lastReport = mi;
	}
}

void Frequency::reportValue(int pin)
{
	int32_t currentTime = millis();
	// Clear the interrupt flag, so that we can read out the counter
	noInterrupts();
	int32_t ticks = _ticks;
	interrupts();
	Firmata.startSysex();
	Firmata.write(FREQUENCY_COMMAND);
	Firmata.write(FREQUENCY_SUBCOMMAND_REPORT);
	Firmata.write(pin);
	Firmata.sendPackedUInt32(currentTime);
	Firmata.sendPackedUInt32(ticks);
	Firmata.endSysex();
}

boolean Frequency::handlePinMode(byte pin, int mode)
{
  int interruptPin = digitalPinToInterrupt(pin);
  if (IS_PIN_DIGITAL(pin) && interruptPin >= 0) 
  {
    if (mode == PIN_MODE_FREQUENCY) {
      return true;
    } else if (pin == _activePin)
	{
      detachInterrupt(interruptPin);
	  _activePin = -1;
    }
  }
  return false;
}

void Frequency::handleCapability(byte pin)
{
  int interrupt = digitalPinToInterrupt(pin);
  if (IS_PIN_DIGITAL(pin) && interrupt >= 0) {
    Firmata.write((byte)PIN_MODE_FREQUENCY);
    Firmata.write((byte)0); // 4 byte clock, 4 byte timestamp
  }
}

void Frequency::reset()
{
	if (_activePin >= 0)
	{
		detachInterrupt(digitalPinToInterrupt(_activePin));
		_activePin = -1;
	}
}
