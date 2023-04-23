
#include <ConfigurableFirmata.h>
#include "ArduinoSleep.h"

void ArduinoSleep::reset()
{
	// Do not reset members, we want to go to sleep even if a reset was executed during disconnect
	_messageReceived = millis();
}

// Code can only be used on ESP32 (others would need a very different implementation for similar features)
#if defined(ESP32)
void ArduinoSleep::EnterSleepMode()
{
	esp_sleep_enable_ext0_wakeup((gpio_num_t)_wakeupPin, _triggerValue); //1 = High, 0 = Low
	// If you were to use ext1, you would use it like
	// esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK,ESP_EXT1_WAKEUP_ANY_HIGH);

	//Go to sleep now
	Serial.println("CPU entering deep sleep mode.");
	esp_deep_sleep_start();
	// The ESP32 will perform a boot when woken up from sleep.
	Serial.println("This will never be printed");
}
#else

#include <avr/sleep.h>
static void WakeUpInterrupt()
{
	sleep_disable();
}

void ArduinoSleep::EnterSleepMode()
{
	sleep_enable();
	byte interruptChannel = digitalPinToInterrupt(_wakeupPin);
	attachInterrupt(interruptChannel, WakeUpInterrupt, _triggerValue ? HIGH : LOW);
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	// Serial.println("CPU entering deep sleep mode.");
	delay(100);
	sleep_cpu();
	delay(10);
	sleep_disable();
	// Serial.println("CPU waking from deep sleep mode.");
	detachInterrupt(interruptChannel);
	_goToSleepAfterDisconnect = false;
}
#endif

void ArduinoSleep::report(bool elapsed)
{
	if (elapsed)
	{
		if (!_goToSleepAfterDisconnect)
		{
			return;
		}

		uint64_t target = (uint64_t)_messageReceived + _sleepTimeout;
		uint64_t now = millis();

		// The first condition is a (slightly incorrect) way to support the wrapping around of the timer value
		if (now < _messageReceived || now > target)
		{
			EnterSleepMode();
		}
	}
}

bool ArduinoSleep::handleSystemVariableQuery(bool write, SystemVariableDataType* data_type, int variable_id, byte pin, SystemVariableError* status, int* value)
{
	if (variable_id == 102)
	{
		int v = *value;
		_goToSleepAfterDisconnect = v != 0;
		_sleepTimeout = 1000 * v; // Timeout in seconds, converted to ms
		if (_goToSleepAfterDisconnect)
		{
			Firmata.sendStringf(F("Sleep mode will be activated after %d ms"), _sleepTimeout);
			_messageReceived = millis();
		}
		*status = SystemVariableError::NoError;
		return true;
	}
	if (variable_id == 103)
	{
		if (digitalPinToInterrupt(pin) < 0)
		{
			*status = SystemVariableError::Error;
			Firmata.sendString(F("Need a valid interrupt pin as wakeup pin"));
			return true;
		}

		if (write)
		{
			_wakeupPin = pin;
			_triggerValue = *value;
			*status = SystemVariableError::NoError;
		}
		else
		{
			// The pin value is not intended as return value, therefore this value is write-only
			*status = SystemVariableError::WriteOnly;
			*value = 0;
		}
		return true;
	}

	return false;
}

