// WifiCachingStream.h

#ifndef _WifiCachingStream_h
#define _WifiCachingStream_h

#include <ConfigurableFirmata.h>
#ifdef ESP32
#include <WiFi.h>
/// <summary>
/// A stream to read data from a TCP connection (server side).
/// </summary>
class WifiCachingStream : public Stream
{
private:
	static constexpr int SendBufferSize = 500;
	int _sd;
	int _port;

	int _connection_sd;

	byte _sendBuffer[SendBufferSize];
	int _sendBufferIndex;

	bool _inSysex;
public:
	WifiCachingStream(int port)
	{
		_sd = -1;
		_port = port;
		_connection_sd = -1;
		_sendBufferIndex = 0;
		_inSysex = false;
	}

	void Init();

	bool Connect();

	int read() override;

	size_t readBytes(char* buffer, size_t length) override;

	size_t write(byte b) override;

	size_t write(const uint8_t* buffer, size_t size) override;

	void maintain();

	int available() override;

	int peek() override;

	void flush() override;

	bool isConnected() const
	{
		return _connection_sd > 0;
	}
};
#endif

#endif
