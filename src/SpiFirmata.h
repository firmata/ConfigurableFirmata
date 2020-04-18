/*
  SpiFirmata.h - Firmata library

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.

*/

#ifndef SpiFirmata_h
#define SpiFirmata_h

#include <ConfigurableFirmata.h>

#include <SPI.h>
#include "FirmataFeature.h"
#include "FirmataReporting.h"

#define SPI_MAX_DEVICES 8
#define MAX_SPI_BUF_SIZE 32

/* Spi data */
struct spi_device_config {
  byte deviceIdChannel;
  byte dataModeBitOrder;
  byte csPinOptions;
  byte csPin;
  boolean used;
};

class SpiFirmata: public FirmataFeature
{
  public:
    SpiFirmata();
    boolean handlePinMode(byte pin, int mode);
    void handleCapability(byte pin);
    boolean handleSysex(byte command, byte argc, byte* argv);
    void reset();
    void report();

  private:
    void handleSpiRequest(byte command, byte argc, byte *argv);
	boolean handleSpiBegin(byte argc, byte *argv);
    boolean handleSpiConfig(byte argc, byte *argv);
    boolean enableSpiPins();
	void handleSpiTransfer(byte argc, byte *argv, boolean dummySend, boolean sendReply);
    void disableSpiPins();
	int getConfigIndexForDevice(byte deviceIdChannel);
	
    spi_device_config config[SPI_MAX_DEVICES];
	bool isSpiEnabled;
};


SpiFirmata::SpiFirmata()
{
  isSpiEnabled = false;
  for (int i = 0; i < SPI_MAX_DEVICES; i++) {
    config[i].deviceIdChannel = -1;
	config[i].csPin = -1;
    config[i].used = false;
  }
}

boolean SpiFirmata::handlePinMode(byte pin, int mode)
{
  if (IS_PIN_SPI(pin)) {
    if (mode == PIN_MODE_SPI) {
      return true;
    } else if (isSpiEnabled) {
      // disable Spi so pins can be used for other functions
      // the following if statements should reconfigure the pins properly
      if (Firmata.getPinMode(pin) == PIN_MODE_SPI) {
        disableSpiPins();
      }
    }
  }
  return false;
}

void SpiFirmata::handleCapability(byte pin)
{
  if (IS_PIN_SPI(pin)) {
    Firmata.write(PIN_MODE_SPI);
    Firmata.write(1);
  }
}

boolean SpiFirmata::handleSysex(byte command, byte argc, byte *argv)
{
  switch (command) {
    case SPI_DATA:
		  if (argc < 1)
		  {
			  Firmata.sendString(F("Error in SPI_DATA command: empty message"));
			  return false;
		  }
        handleSpiRequest(argv[0], argc - 1, argv + 1);
        return true;
  }
  return false;
}

void SpiFirmata::handleSpiRequest(byte command, byte argc, byte *argv)
{
  switch (command) {
	  case SPI_BEGIN:
	    handleSpiBegin(argc, argv);
		break;
	  case SPI_DEVICE_CONFIG:
	    handleSpiConfig(argc, argv);
		break;
	  case SPI_END:
	    disableSpiPins();
		break;
	  case SPI_READ:
	    handleSpiTransfer(argc, argv, true, true);
		break;
	  case SPI_WRITE:
	    handleSpiTransfer(argc, argv, false, false);
		break;
	  case SPI_TRANSFER:
	    handleSpiTransfer(argc, argv, false, true);
		break;
	  default:
	    Firmata.sendString(F("Unknown SPI command: "), command);
		break;
  }
}

void SpiFirmata::handleSpiTransfer(byte argc, byte *argv, boolean dummySend, boolean sendReply)
{
	if (!isSpiEnabled)
	{
		Firmata.sendString(F("SPI not enabled."));
		return;
	}
	byte data[MAX_SPI_BUF_SIZE];
	// Make sure we have enough data. No data bytes is only allowed in read-only mode
	if (dummySend ? argc < 4 : argc < 6) {
		Firmata.sendString(F("Not enough data in SPI message"));
		return;
	}
	
	int index = getConfigIndexForDevice(argv[0]);
	if (index < 0) {
		Firmata.sendString(F("SPI_TRANSFER: Unknown deviceId specified: "), argv[0]);
		return;
	}
	
	int j = 0;
	// In read-only mode set buffer to 0, otherwise fill buffer from request
	if (dummySend) {
		for (byte i = 0; i < argv[3]; i += 1) {
          data[j++] = 0;
	  }
	} else {
	  for (byte i = 4; i < argc; i += 2) {
          data[j++] = argv[i] + (argv[i + 1] << 7);
	  }
	}
	Firmata.sendString(F("In handleSpiTransfer : Real send"));
	
	digitalWrite(config[index].csPin, LOW);
	SPI.transfer(data, j); 
	if (argv[2] != 0)
	{
		// Default is deselect, so only skip this if the value is 0
		digitalWrite(config[index].csPin, HIGH);
	}
	Firmata.sendString(F("In handleSpiTransfer : ???"));
	if (sendReply) {
		Firmata.sendString(F("In handleSpiTransfer : Send reply"));
	  Firmata.startSysex();
	  Firmata.write(SPI_DATA);
	  Firmata.write(SPI_REPLY);
	  Firmata.write(argv[0]);
	  Firmata.write(argv[1]);
	  Firmata.write(j);
	  for (int i = 0; i < j; i++)
	  {
		  Firmata.sendValueAsTwo7bitBytes(data[i]);
	  }
	  Firmata.endSysex();
	}
}

boolean SpiFirmata::handleSpiConfig(byte argc, byte *argv)
{
	if (argc < 10) {
		Firmata.sendString(F("Not enough data in SPI_DEVICE_CONFIG message"));
		return false;
	}
	Firmata.sendString("SpiConfigMessage");
	for (int i = 0; i < argc; i++)
	{
		Firmata.sendString(String(argv[i], HEX).c_str());
	}
	
  int index = -1; // the index where the new device will be added
  for (int i = 0; i < SPI_MAX_DEVICES; i++) {
    if (config[i].deviceIdChannel == argv[0]) {
		index = i; // This device exists already
	}
  }
  
  if (index == -1)
  {
	  for (int i = 0; i < SPI_MAX_DEVICES; i++) {
		if (config[i].used == false) {
			index = i;
		}
	  }
  }
  if (index == -1)
  {
	  Firmata.sendString(F("SPI_DEVICE_CONFIG: Max number of devices exceeded"));
	  return false;
  }
  
  // Check word size. Must be 0 (default) or 8.
  if (argv[7] != 0 && argv[7] != 8)
  {
	  Firmata.sendString(F("SPI_DEVICE_CONFIG: Only 8 bit words supported"));
	  return false;
  }
  
  byte deviceIdChannel = argv[0];
  if (deviceIdChannel & 0x3 != 0)
  {
	  Firmata.sendString(F("SPI_DEVICE_CONFIG: Only channel 0 supported: "), deviceIdChannel);
	  return false;
  }
  
  if (argv[1] != 1)
  {
	  Firmata.sendString(F("Only BitOrder = 1 and dataMode = 0 supported"));
	  return false;
  }
  
  config[index].deviceIdChannel = deviceIdChannel;
  config[index].dataModeBitOrder = argv[1];
  // Max speed ignored for now
  config[index].csPinOptions = argv[8];
  config[index].csPin = argv[9];
  config[index].used = true;
  Firmata.sendString(F("Configured settings for device Id "), deviceIdChannel >> 3);
}

int SpiFirmata::getConfigIndexForDevice(byte deviceIdChannel)
{
  for (int i = 0; i < SPI_MAX_DEVICES; i++) {
    if (config[i].deviceIdChannel == deviceIdChannel) {
		return i;
	}
  }
  return -1;
}

boolean SpiFirmata::handleSpiBegin(byte argc, byte *argv)
{
  if (!isSpiEnabled) {
	  // Only channel 0 supported
    if (argc != 1 || *argv != 0) {
		Firmata.sendString(F("SPI_BEGIN: Only channel 0 supported"));
		return false;
	}
    enableSpiPins();
	Firmata.sendString("SPI_BEGIN");
	SPI.begin();

  }
  return isSpiEnabled;
}

boolean SpiFirmata::enableSpiPins()
{
  if (Firmata.getPinMode(PIN_SPI_MISO) == PIN_MODE_IGNORE) {
        return false;
  }
  // mark pins as Spi so they are ignore in non Spi data requests
  Firmata.setPinMode(PIN_SPI_MISO, PIN_MODE_SPI);
  pinMode(PIN_SPI_MISO, INPUT);
  
  if (Firmata.getPinMode(PIN_SPI_MOSI) == PIN_MODE_IGNORE) {
        return false;
  }
  // mark pins as Spi so they are ignore in non Spi data requests
  Firmata.setPinMode(PIN_SPI_MOSI, PIN_MODE_SPI);
  pinMode(PIN_SPI_MOSI, OUTPUT);
  
  if (Firmata.getPinMode(PIN_SPI_SCK) == PIN_MODE_IGNORE) {
        return false;
  }
  // mark pins as Spi so they are ignore in non Spi data requests
  Firmata.setPinMode(PIN_SPI_SCK, PIN_MODE_SPI);
  pinMode(PIN_SPI_SCK, OUTPUT);
  
  isSpiEnabled = true;
  return true;
}

/* disable the Spi pins so they can be used for other functions */
void SpiFirmata::disableSpiPins()
{
  isSpiEnabled = false;
  Firmata.sendString("SPI_END");
  SPI.end();
}

void SpiFirmata::reset()
{
  if (isSpiEnabled) {
    disableSpiPins();
  }
}

void SpiFirmata::report()
{
}

#endif /* SpiFirmata_h */
