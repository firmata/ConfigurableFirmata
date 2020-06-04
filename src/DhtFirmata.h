/*
  DhtFirmata.h - Firmata library

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.

*/

#ifndef DhtFirmata_h
#define DhtFirmata_h

#include <ConfigurableFirmata.h>

#include "FirmataFeature.h"
#include "FirmataReporting.h"

#define DHTSENSOR_RESPONSE (0)
#define DHTSENSOR_ATTACH_DHT11 (0x01)
#define DHTSENSOR_ATTACH_DHT22 (0x02)
#define DHTSENSOR_DETACH (0x03)

#include <DHT.h>

class DhtFirmata: public FirmataFeature
{
  public:
    DhtFirmata();
    boolean handlePinMode(byte pin, int mode);
    void handleCapability(byte pin);
    boolean handleSysex(byte command, byte argc, byte* argv);
    void reset();
    void report();

  private:
    void performDhtTransfer(byte command, byte argc, byte *argv);
    void disableDht();
    bool _isDhtEnabled;
    int _dhtPin; // Only one sensor supported
    DHT* _dht;
};


DhtFirmata::DhtFirmata()
{
  _isDhtEnabled = false;
  _dhtPin = -1;
}

boolean DhtFirmata::handlePinMode(byte pin, int mode)
{
  if (IS_PIN_DIGITAL(pin)) {
    if (mode == PIN_MODE_DHT) {
      return true;
    }
	else if (_isDhtEnabled) 
	{
		disableDht();
	}
  }
  return false;
}

void DhtFirmata::handleCapability(byte pin)
{
  if (IS_PIN_DIGITAL(pin)) {
    Firmata.write(PIN_MODE_DHT);
    Firmata.write(64); // 2x 32 Bit data per measurement
  }
}

boolean DhtFirmata::handleSysex(byte command, byte argc, byte *argv)
{
  switch (command) {
    case DHTSENSOR_DATA:
		  if (argc < 2)
		  {
			  Firmata.sendString(F("Error in DHT command: Not enough parameters"));
			  return false;
		  }
        performDhtTransfer(argv[0], argc - 1, argv + 1);
        return true;

  }
  return false;
}

void DhtFirmata::performDhtTransfer(byte command, byte argc, byte *argv)
{
	if (command == DHTSENSOR_DETACH)
	{
		disableDht();
		return;
	}
	// command byte: DHT Type
	byte dhtType = command;
	// The proposed protocol specification https://github.com/firmata/protocol/pull/106/commits/9389c89d3d7998dc2bd703f8c796e6a7c03f2551
	// allows these values, too.
	if (dhtType == 0x01)
	{
		dhtType = 11;
	}
	else if (dhtType == 0x02)
	{
		dhtType = 22;
	}
	
	// first byte: pin
	byte pin = argv[0];
	if (!_isDhtEnabled)
	{
		_dhtPin = pin;
		_isDhtEnabled = true;
		_dht = new DHT(pin, dhtType);
		_dht->begin();
	}
	else if (_dhtPin != pin)
	{
		// Switching the pin unfortunately requires re-loading the DHT class
		delete _dht;
		_dht = nullptr;
		_dht = new DHT(pin, dhtType);
		_dht->begin();
	}
	
	// Accuracy of the sensor is very limited, only 8 bit humidity and maybe 10 bit for temperature
	short humidity = (short)_dht->readHumidity() * 10;
	short temperature = (short)(_dht->readTemperature() * 10);
	
	Firmata.startSysex();
	Firmata.write(DHTSENSOR_DATA);
	Firmata.write(DHTSENSOR_RESPONSE);
	Firmata.write(pin);
	Firmata.write(temperature & 0x7f);
	Firmata.write((temperature >> 7) & 0x7f);
	
	Firmata.write(humidity & 0x7f);
	Firmata.write((humidity >> 7) & 0x7f);
	Firmata.endSysex();
}

void DhtFirmata::disableDht()
{
	delete _dht;
	_dht = NULL;
	_isDhtEnabled = false;
}

void DhtFirmata::reset()
{
  if (_isDhtEnabled) {
	disableDht();
  }
}

void DhtFirmata::report()
{
}

#endif 
