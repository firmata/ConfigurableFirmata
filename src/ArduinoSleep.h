// ArduinoSleep.h

#pragma once
#include <ConfigurableFirmata.h>
#include "FirmataFeature.h"

class ArduinoSleep : public FirmataFeature
{
private:
	bool _goToSleepAfterDisconnect;
	uint32_t _sleepTimeout;
	uint32_t _messageReceived;
	byte _wakeupPin;
	byte _triggerValue;

public:
	ArduinoSleep(byte wakeupPin, byte triggerValue)
		: FirmataFeature()
	{
		_wakeupPin = wakeupPin;
		_triggerValue = triggerValue;
		_goToSleepAfterDisconnect = false;
		_sleepTimeout = 0;
		_messageReceived = 0;
	}

	virtual boolean handleSysex(byte command, byte argc, byte* argv) override
	{
		// Empty
		return false;
	}

	void reset() override;

	void report(bool elapsed) override;

	void handleCapability(byte pin) override
	{
		// Empty
	}

	boolean handlePinMode(byte pin, int mode) override
	{
		// Empty
		return false;
	}
	
	bool handleSystemVariableQuery(bool write, SystemVariableDataType* data_type, int variable_id, byte pin, SystemVariableError* status, int* value) override;

private:
	void EnterSleepMode();
};
