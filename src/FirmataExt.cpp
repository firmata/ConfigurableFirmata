/*
  FirmataExt.h - Firmata library
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
#include "FirmataExt.h"

FirmataExt *FirmataExtInstance;

void handleSetPinModeCallback(byte pin, int mode)
{
  if (!FirmataExtInstance->handlePinMode(pin, mode) && mode != PIN_MODE_IGNORE) {
    Firmata.sendString(F("Unknown pin mode")); 
  }
}

void handleSysexCallback(byte command, byte argc, byte* argv)
{
  if (!FirmataExtInstance->handleSysex(command, argc, argv)) {
    Firmata.sendStringf(F("Unhandled sysex command: 0x%x (len: %d)"), (int)command, (int)argc);
  }
}

FirmataExt::FirmataExt()
{
  FirmataExtInstance = this;
  Firmata.attach(SET_PIN_MODE, handleSetPinModeCallback);
  Firmata.attach((byte)START_SYSEX, handleSysexCallback);
    for (int i = 0; i < MAX_FEATURES; i++)
    {
        features[i] = nullptr;
    }
  numFeatures = 0;
}

void FirmataExt::handleCapability(byte pin)
{

}

boolean FirmataExt::handlePinMode(byte pin, int mode)
{
  boolean known = false;
  for (byte i = 0; i < numFeatures; i++) {
    known |= features[i]->handlePinMode(pin, mode);
  }
  return known;
}

boolean FirmataExt::handleSysex(byte command, byte argc, byte* argv)
{
  switch (command) {

    case PIN_STATE_QUERY:
      if (argc > 0) {
        byte pin = argv[0];
        if (pin < TOTAL_PINS) {
          Firmata.write(START_SYSEX);
          Firmata.write(PIN_STATE_RESPONSE);
          Firmata.write(pin);
          Firmata.write(Firmata.getPinMode(pin));
          int pinState = Firmata.getPinState(pin);
          Firmata.write((byte)pinState & 0x7F);
          if (pinState & 0xFF80) Firmata.write((byte)(pinState >> 7) & 0x7F);
          if (pinState & 0xC000) Firmata.write((byte)(pinState >> 14) & 0x7F);
          Firmata.write(END_SYSEX);
          return true;
        }
      }
      break;
    case CAPABILITY_QUERY:
      Firmata.write(START_SYSEX);
      Firmata.write(CAPABILITY_RESPONSE);
      for (byte pin = 0; pin < TOTAL_PINS; pin++) {
        if (Firmata.getPinMode(pin) != PIN_MODE_IGNORE) {
          for (byte i = 0; i < numFeatures; i++) {
            features[i]->handleCapability(pin);
          }
        }
        Firmata.write(127);
      }
      Firmata.write(END_SYSEX);
      return true;
    case SYSTEM_VARIABLE:
	    {
		    if (argc < 11)
		    {
                Firmata.sendString(F("Not enough bytes in SYSTEM_VARIABLE message"));
                return false;
		    }
            bool write = argv[0];
            SystemVariableDataType data_type = (SystemVariableDataType)argv[1];

            SystemVariableError status = SystemVariableError::UnknownVariable; // Input value is irrelevant, so set to default reply value
            int variable_id = Firmata.decodePackedUInt14(argv + 3);
            byte pin = argv[5];
            int value = (int)Firmata.decodePackedUInt32(argv + 6);
            
			// Test basic variables first (implemented on ourselves)
			if (!handleSystemVariableQuery(write, &data_type, variable_id, pin, &status, &value))
			{
				for (byte i = 0; i < numFeatures; i++) 
				{
					if (features[i]->handleSystemVariableQuery(write, &data_type, variable_id, pin, &status, &value))
					{
						break;
					}
				}
			}

            Firmata.write(START_SYSEX);
            Firmata.write(SYSTEM_VARIABLE);
            Firmata.write((byte)write);
            Firmata.write((byte)data_type);
            Firmata.write((byte)status);
            Firmata.sendPackedUInt14(variable_id);
            Firmata.write(pin);
            Firmata.sendPackedUInt32(value);
            Firmata.write(END_SYSEX);
	    }
        return true;
    default:
      for (byte i = 0; i < numFeatures; i++) {
        if (features[i]->handleSysex(command, argc, argv)) {
			return true;
        }
      }
      break;
  }
  return false;
}

void FirmataExt::addFeature(FirmataFeature &capability)
{
  if (numFeatures < MAX_FEATURES) {
    features[numFeatures++] = &capability;
  }
}

void FirmataExt::reset()
{
  for (byte i = 0; i < numFeatures; i++) {
    features[i]->reset();
  }
}

void FirmataExt::report(bool elapsed)
{
  for (byte i = 0; i < numFeatures; i++) {
    features[i]->report(elapsed);
  }
}

bool FirmataExt::handleSystemVariableQuery(bool write, SystemVariableDataType* data_type, int variable_id, byte pin, SystemVariableError* status, int* value)
{
	// This handles the basic variables that are system and component independent
	if (variable_id == 0)
	{
        // System variable availability check: This always returns 1
		*value = 1;
		*data_type = SystemVariableDataType::Int;
		*status = SystemVariableError::NoError;
		return true;
	}
	if (variable_id == 1)
	{
        // Max Sysex messae size
		*value = MAX_DATA_BYTES;
		*data_type = SystemVariableDataType::Int;
		*status = SystemVariableError::NoError;
		return true;
	}
    if (variable_id == 2)
    {
        // Input buffer size
#if defined(LARGE_MEM_DEVICE)
        *value = LARGE_MEM_RCV_BUF_SIZE;
#else
        *value = MAX_DATA_BYTES;
#endif
        *data_type = SystemVariableDataType::Int;
        *status = SystemVariableError::NoError;
        return true;
    }

	return false;
}

