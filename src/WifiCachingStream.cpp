// 
// 
// 

#include <ConfigurableFirmata.h>
#include "WifiCachingStream.h"

#ifdef ESP32

#include <sys/poll.h>
#include "EspNetworkFunctions.h"

void WifiCachingStream::Init()
{
	// Also initializes other background processes
	// ESP_ERROR_CHECK(esp_event_loop_create_default());
	if (!network_create_listening_socket(&_sd, _port, 1))
	{
		Firmata.sendStringf(F("Error opening listening socket."));
	}
}

bool WifiCachingStream::Connect()
{
	if (_connection_sd >= 0)
	{
		return true;
	}

	if (_sd < 0)
	{
		return false;
	}

	auto result = network_wait_for_connection(_sd, &_connection_sd, nullptr, true);
	if (result != E_NETWORK_RESULT_OK)
	{
		_connection_sd = -1;
		return false;
	}

	if (_connection_sd >= 0)
	{
		Serial.println("New client connected");
		Firmata.resetParser();
		WiFi.setSleep(false);
		return true;
	}
	else
	{
		Serial.println("Client disconnected - entering WiFi low-power mode");
		// Low-power mode significantly increases round-trip time, but when nobody
		// is connected, that's ok.
		// WiFi.setSleep(true);
		Firmata.resetParser(); // clear any partial message from the parser when the connection is dropped.

		return false;
	}
}

int WifiCachingStream::read()
{
	// This method is unimplemented, as we only need readBytes with this class, for better performance.
	ESP_LOGE("[NET]", "FATAL: WifiCachingStream::read not implemented");
	throw "NOTIMPLEMENTED"; // Just crash.
}

int WifiCachingStream::available()
{
	// available returns 0 in case of an error or nothing to do.
	if (_connection_sd < 0)
	{
		return 0;
	}

	int p = network_poll(&_connection_sd);
	if (p < 0)
	{
		network_close_socket(&_connection_sd);
		Serial.println(F("Connection dropped while testing for bytes"));
		Firmata.resetParser();
		return 0;
	}

	return p;
}

int WifiCachingStream::peek()
{
	return -1;
}

void WifiCachingStream::flush()
{
	// probably nothing to do
}



size_t WifiCachingStream::readBytes(char* buffer, size_t length)
{
	size_t received = 0;
	auto result = network_recv_non_blocking(_connection_sd, buffer, (int)length, &received);
	if (received >= 1)
	{
		return received;
	}
	if (result == E_NETWORK_RESULT_FAILED)
	{
		network_close_socket(&_connection_sd);
		Serial.println(F("Connection dropped"));
	}

	return -1;
}

size_t WifiCachingStream::write(byte b)
{
	if (b == START_SYSEX)
	{
		_inSysex = true;
	}
	else if (b == END_SYSEX)
	{
		_inSysex = false;
	}

	_sendBuffer[_sendBufferIndex] = b;
	_sendBufferIndex++;
	// Send when the buffer is full or we're not in a sysex message or at the end of it.
	if (_sendBufferIndex >= SendBufferSize || _inSysex == false)
	{
		int ret = network_send(_connection_sd, _sendBuffer, _sendBufferIndex);
		_sendBufferIndex = 0;
		return ret >= 1;
	}
	return 1;
}

size_t WifiCachingStream::write(const uint8_t* buffer, size_t size)
{
	return network_send(_connection_sd, buffer, size);
}


void WifiCachingStream::maintain()
{
	Connect();
	yield();
}

#endif
