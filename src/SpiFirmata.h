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
#include "Encoder7Bit.h"

#define SPI_BEGIN               0x00 // Initialize the SPI bus for the given channel
#define SPI_DEVICE_CONFIG       0x01
#define SPI_TRANSFER            0x02
#define SPI_WRITE               0x03
#define SPI_READ                0x04
#define SPI_REPLY               0x05
#define SPI_END                 0x06
#define SPI_WRITE_ACK           0x07

#define SPI_SEND_NO_REPLY 0
#define SPI_SEND_NORMAL_REPLY 1
#define SPI_SEND_EMPTY_REPLY 2

#define SPI_MAX_DEVICES 8
#define MAX_SPI_BUF_SIZE 32

/* Spi data */
struct spi_device_config {
  byte deviceIdChannel;
  byte csPinOptions;
  byte csPin;
  boolean packedData;
  SPISettings spi_settings;
  boolean used;
public:
	spi_device_config()
		: spi_settings()
	{
		deviceIdChannel = 0;
		csPinOptions = 0;
		csPin = -1;
		packedData = false;
		used = false;
	}
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
	void handleSpiTransfer(byte argc, byte *argv, boolean dummySend, int sendReply);
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
	config[i].packedData = false;
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
	    handleSpiTransfer(argc, argv, true, SPI_SEND_NORMAL_REPLY);
		break;
	  case SPI_WRITE:
	    handleSpiTransfer(argc, argv, false, SPI_SEND_NO_REPLY);
		break;
	  case SPI_WRITE_ACK:
		  handleSpiTransfer(argc, argv, false, SPI_SEND_EMPTY_REPLY);
		  break;
	  case SPI_TRANSFER:
	    handleSpiTransfer(argc, argv, false, SPI_SEND_NORMAL_REPLY);
		break;
	  default:
	    Firmata.sendString(F("Unknown SPI command: "), command);
		break;
  }
}

void SpiFirmata::handleSpiTransfer(byte argc, byte *argv, boolean dummySend, int sendReply)
{
	if (!isSpiEnabled)
	{
		Firmata.sendString(F("SPI not enabled."));
		return;
	}
	byte data[MAX_DATA_BYTES];
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
	
	int bytesToSend = 0;
	// In read-only mode set buffer to 0, otherwise fill buffer from request
	if (dummySend) {
		memset(data, 0, argv[3]);
		bytesToSend = argv[3];
	} else 
	{
		if (config[index].packedData)
		{
			bytesToSend = num7BitOutbytes(argc - 4);
			if (bytesToSend > MAX_DATA_BYTES)
			{
				Firmata.sendString(F("SPI_TRANSFER: Send buffer not large enough"));
				return;
			}
			Encoder7BitClass::readBinary(bytesToSend, argv + 4, data);
		}
		else
		{
			for (byte i = 4; i < argc; i += 2) 
			{
				data[bytesToSend++] = argv[i] + (argv[i + 1] << 7);
			}
		}
	}

	// If we just need to send a confirmation message, we can do so now, as we have already copied the input buffer (and we're running synchronously, anyway)
	if (sendReply == SPI_SEND_EMPTY_REPLY)
	{
		byte reply[7];
		reply[0] = START_SYSEX;
		reply[1] = SPI_DATA;
		reply[2] = SPI_REPLY;
		reply[3] = argv[0];
		reply[4] = argv[1];
		reply[5] = 0;
		reply[6] = END_SYSEX;
		Firmata.write(reply, 7);
	}

	if (config[index].csPin != -1)
	{
		digitalWrite(config[index].csPin, LOW);
	}

	SPI.beginTransaction(config[index].spi_settings);
	SPI.transfer(data, bytesToSend);
	SPI.endTransaction();
	if (argv[2] != 0)
	{
		// Default is deselect, so only skip this if the value is 0
		digitalWrite(config[index].csPin, HIGH);
	}
	if (sendReply == SPI_SEND_NORMAL_REPLY) {
	  Firmata.startSysex();
	  Firmata.write(SPI_DATA);
	  Firmata.write(SPI_REPLY);
	  Firmata.write(argv[0]);
	  Firmata.write(argv[1]);
	  Firmata.write((byte)bytesToSend); // the bytes received is always equal to the bytes sent for SPI
		if (config[index].packedData)
		{
			Encoder7BitClass encoder;
			encoder.startBinaryWrite();
			for (int i = 0; i < bytesToSend; i++) 
			{
				encoder.writeBinary(data[i]);
			}
			encoder.endBinaryWrite();
		}
		else
		{
			for (int i = 0; i < bytesToSend; i++)
			{
				Firmata.sendValueAsTwo7bitBytes(data[i]);
			}
		}
	  Firmata.endSysex();
	}
}

boolean SpiFirmata::handleSpiConfig(byte argc, byte* argv)
{
	if (argc < 10) {
		Firmata.sendString(F("Not enough data in SPI_DEVICE_CONFIG message"));
		return false;
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
	if ((deviceIdChannel & 0x3) != 0)
	{
		Firmata.sendString(F("SPI_DEVICE_CONFIG: Only channel 0 supported: "), deviceIdChannel);
		return false;
	}

	spi_device_config& cfg = config[index];
	cfg.deviceIdChannel = deviceIdChannel;
	int bitOrder = argv[1] & 0x1;
	int dataMode = (argv[1] >> 1) & 0x3;
	cfg.packedData = (argv[1] >> 3) & 0x1; // this is only supported for the default word length, but we're not supporting anything else anyway
	uint32_t speed = Firmata.decodePackedUInt32(argv + 2);
	cfg.csPinOptions = argv[8];
	cfg.csPin = argv[9];
	cfg.used = true;
	if (speed == 0)
	{
		speed = 5000000;
	}
	cfg.spi_settings = SPISettings(speed, bitOrder == 1 ? MSBFIRST : LSBFIRST, dataMode);
	if ((cfg.csPinOptions & 0x1) == 0)
	{
		cfg.csPin = -1;
	}
	else
	{
		Firmata.setPinMode(cfg.csPin, PIN_MODE_OUTPUT);
		pinMode(cfg.csPin, OUTPUT);
	}

	Firmata.sendStringf(F("New SPI device %d allocated with index %d and CS %d, clock speed %d Hz"), deviceIdChannel, index, config[index].csPin, speed);
	return true;
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

  	if (!enableSpiPins())
  	{
		Firmata.sendString(F("Error enabling SPI mode"));
		return false;
  	}

	SPI.begin();
	Firmata.sendString(F("SPI.begin()"));
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
  SPI.end();
  Firmata.sendString(F("SPI.end()"));
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
