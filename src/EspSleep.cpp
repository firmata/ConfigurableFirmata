
#include <ConfigurableFirmata.h>
#include "EspSleep.h"

// Code can only be used on ESP32 (others would need a very different implementation for similar features)
#ifdef ESP32
void EspSleep::reset()
{
	// Do not reset members, we want to go to sleep even if a reset was executed during disconnect
	_messageReceived = millis();
}

void EspSleep::EnterSleepMode()
{
	esp_sleep_enable_ext0_wakeup((gpio_num_t)_wakeupPin, _triggerValue); //1 = High, 0 = Low
	// If you were to use ext1, you would use it like
	// esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK,ESP_EXT1_WAKEUP_ANY_HIGH);

	//Go to sleep now
	Serial.println("CPU entering deep sleep mode.");
	esp_deep_sleep_start();
	Serial.println("This will never be printed");
}

void EspSleep::report(bool elapsed)
{
	if (elapsed)
	{
		if (!_goToSleepAfterDisconnect)
		{
			return;
		}

		uint64_t target = (uint64_t)_messageReceived + _sleepTimeout;
		uint64_t now = millis();

		if (now < _lastDisconnect || now > target)
		{
			EnterSleepMode();
		}
	}
}

bool EspSleep::handleSystemVariableQuery(bool write, SystemVariableDataType* data_type, int variable_id, byte pin, SystemVariableError* status, int* value)
{
	if (variable_id == 102)
	{
		_goToSleepAfterDisconnect = value != 0;
		_sleepTimeout = 1000 * 60 * value; // Timeout in minutes, converted to ms
		if (_goToSleepAfterDisconnect)
		{
			Firmata.sendStringf(F("Sleep mode will be activated after %d ms"), _sleepTimeout);
			_messageReceived = millis();
		}
		return true;
	}
	return __super::handleSystemVariableQuery(write, data_type, variable_id, pin, status, value);
}


#endif
