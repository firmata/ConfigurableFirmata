/*
  OneWireFirmata.cpp - Firmata library
  Copyright (C) 2012-2013 Norbert Truchsess. All rights reserved.
  Copyright (C) 2016 Jeff Hoefs. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.

  Last updated by Jeff Hoefs: January 23rd, 2016
*/

#include <ConfigurableFirmata.h>
#include "OneWireFirmata.h"
#include "Encoder7Bit.h"

OneWireFirmata::OneWireFirmata()
{
  for (int i = 0; i < TOTAL_PINS; i++) {
    pinOneWire[i].device = NULL;
    pinOneWire[i].power = false;
  }
}

boolean OneWireFirmata::handlePinMode(byte pin, int mode)
{
  if (IS_PIN_DIGITAL(pin) && mode == PIN_MODE_ONEWIRE) {
    oneWireConfig(pin, ONEWIRE_POWER);
    return true;
  }
  return false;
}

void OneWireFirmata::handleCapability(byte pin)
{
  if (IS_PIN_DIGITAL(pin)) {
    Firmata.write(PIN_MODE_ONEWIRE);
    Firmata.write(1);
  }
}

void OneWireFirmata::oneWireConfig(byte pin, boolean power)
{
  ow_device_info *info = &pinOneWire[pin];
  if (info->device == NULL) {
    info->device = new OneWire(pin);
  }
  info->power = power;
}

boolean OneWireFirmata::handleSysex(byte command, byte argc, byte* argv)
{
  if (command == ONEWIRE_DATA) {
    Encoder7BitClass encoder;
    if (argc > 1) {
      byte subcommand = argv[0];
      byte pin = argv[1];
      ow_device_info *info = &pinOneWire[pin];
      OneWire *device = info->device;
      if (device || subcommand == ONEWIRE_CONFIG_REQUEST) {
        switch (subcommand) {
          case ONEWIRE_SEARCH_REQUEST:
          case ONEWIRE_SEARCH_ALARMS_REQUEST:
            {
              device->reset_search();
              Firmata.write(START_SYSEX);
              Firmata.write(ONEWIRE_DATA);
              boolean isAlarmSearch = (subcommand == ONEWIRE_SEARCH_ALARMS_REQUEST);
              Firmata.write(isAlarmSearch ? (byte)ONEWIRE_SEARCH_ALARMS_REPLY : (byte)ONEWIRE_SEARCH_REPLY);
              Firmata.write(pin);
              encoder.startBinaryWrite();
              byte addrArray[8];
              while (isAlarmSearch ? device->search(addrArray, false) : device->search(addrArray)) {
                for (int i = 0; i < 8; i++) {
                  encoder.writeBinary(addrArray[i]);
                }
              }
              encoder.endBinaryWrite();
              Firmata.write(END_SYSEX);
              break;
            }
          case ONEWIRE_CONFIG_REQUEST:
            {
              if (argc == 3 && Firmata.getPinMode(pin) != PIN_MODE_IGNORE) {
                Firmata.setPinMode(pin, PIN_MODE_ONEWIRE);
                oneWireConfig(pin, argv[2]); // this calls oneWireConfig again, this time setting the correct config (which doesn't cause harm though)
              } else {
                return false;
              }
              break;
            }
          default:
            {
              if (subcommand & ONEWIRE_RESET_REQUEST_BIT) {
                device->reset();
              }
              if (subcommand & ONEWIRE_SKIP_REQUEST_BIT) {
                device->skip();
              }
              if (subcommand & ONEWIRE_WITHDATA_REQUEST_BITS) {
                int numBytes = num7BitOutbytes(argc - 2);
                int numReadBytes = 0;
                int correlationId;
                argv += 2;
                Encoder7BitClass::readBinary(numBytes, argv, argv); //decode inplace

                if (subcommand & ONEWIRE_SELECT_REQUEST_BIT) {
                  if (numBytes < 8) break;
                  device->select(argv);
                  argv += 8;
                  numBytes -= 8;
                }

                if (subcommand & ONEWIRE_READ_REQUEST_BIT) {
                  if (numBytes < 4) break;
                  numReadBytes = *((int*)argv);
                  argv += 2;
                  correlationId = *((int*)argv);
                  argv += 2;
                  numBytes -= 4;
                }

                if (subcommand & ONEWIRE_DELAY_REQUEST_BIT) {
                  if (numBytes < 4) break;
                  Firmata.delayTask(*((long*)argv));
                  argv += 4;
                  numBytes -= 4;
                }

                if (subcommand & ONEWIRE_WRITE_REQUEST_BIT) {
                  for (int i = 0; i < numBytes; i++) {
                    info->device->write(argv[i], info->power);
                  }
                }

                if (numReadBytes > 0) {
                  Firmata.write(START_SYSEX);
                  Firmata.write(ONEWIRE_DATA);
                  Firmata.write(ONEWIRE_READ_REPLY);
                  Firmata.write(pin);
                  encoder.startBinaryWrite();
                  encoder.writeBinary(correlationId & 0xFF);
                  encoder.writeBinary((correlationId >> 8) & 0xFF);
                  for (int i = 0; i < numReadBytes; i++) {
                    encoder.writeBinary(device->read());
                  }
                  encoder.endBinaryWrite();
                  Firmata.write(END_SYSEX);
                }
              }
            }
        }
      }
      return true;
    }
  }
  return false;
}

void OneWireFirmata::reset()
{
  for (int i = 0; i < TOTAL_PINS; i++) {
    if (pinOneWire[i].device) {
      free(pinOneWire[i].device);
      pinOneWire[i].device = NULL;
    }
    pinOneWire[i].power = false;
  }
}
