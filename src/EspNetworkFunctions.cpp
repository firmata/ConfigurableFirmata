// Network wrapping primitives

#include "ConfigurableFirmata.h"
#include "EspNetworkFunctions.h"

#ifdef ESP32
#include <lwip/sockets.h>
#include <sys/poll.h>

#include "esp_wifi.h"
#include <WiFi.h>

const char* NETWORK_TAG = "[NET]";
/// <summary>
/// Checks whether there's data on the socket
/// </summary>
/// <param name="socket">Reference to the socket to probe</param>
/// <returns>A value > 0 when bytes are available, a value < 0 on error and = 0 when no data is available. In case of an error, the socket is closed.</returns>
int network_poll(int32_t* socket)
{
	if (*socket < 0)
	{
		return -1;
	}

	pollfd pd;
	pd.fd = *socket;
	pd.events = POLLIN;
	pd.revents = 0;
	int ret = poll(&pd, 1, 0);
	if (pd.revents & POLLIN)
	{
		return 1;
	}
	if (ret < 0)
	{
		network_close_socket(socket);
		return -1;
	}

	return 0;
}

//------------------------------------------------------------------------------------------------
network_result_t network_recv_non_blocking(int32_t sd, char* buff, int32_t maxlen, int32_t* rxLen)
{
	if (sd < 0) return E_NETWORK_RESULT_FAILED;
	*rxLen = recv(sd, buff, maxlen, 0);
	if (*rxLen > 0)
	{
		return E_NETWORK_RESULT_OK;
	}
	else
	{
		if (errno != EAGAIN)
		{
			return E_NETWORK_RESULT_FAILED;
		}
	}

	*rxLen = 0;
	return E_NETWORK_RESULT_CONTINUE;
}



network_result_t network_send(int32_t socket, byte b)
{
	return (network_result_t)send(socket, &b, 1, 0);
}

network_result_t network_send(int32_t socket, byte b, bool isLast)
{
	return (network_result_t)send(socket, &b, 1, isLast ? 0 : MSG_MORE);
}

network_result_t network_send(int32_t socket, const byte* data, size_t length)
{
	return (network_result_t)send(socket, data, length, 0);
}

//--------------------------------------------------------------------------------------------
network_result_t network_wait_for_connection(int32_t listeningSocket, int32_t* connectionSocket, uint32_t* ip_addr, bool nonblocking)
{
	struct sockaddr_in	sClientAddress;
	socklen_t  in_addrSize;

	// accepts a connection from a TCP client, if there is any, otherwise returns EAGAIN
	*connectionSocket = accept(listeningSocket, (struct sockaddr*)&sClientAddress, (socklen_t*)&in_addrSize);
	int32_t _sd = *connectionSocket;
	if (_sd < 0) {
		if (errno == EAGAIN) {
			return E_NETWORK_RESULT_CONTINUE;
		}
		// error
		return E_NETWORK_RESULT_FAILED;
	}
	if (ip_addr) {
		*ip_addr = WiFi.localIP();
	}

	// enable non-blocking mode if not data channel connection
	uint32_t option = fcntl(_sd, F_GETFL, 0);
	if (nonblocking) {
		option |= O_NONBLOCK;
	}
	fcntl(_sd, F_SETFL, option);
	// This sends all data immediately, which sligly reduces round trip time, but is not the general solution,
	// because to many packets with 1 byte get transmitted (the way we do our sending)
	// int flag = 1;
	// setsockopt(_sd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));

	// client connected, so go on
	return E_NETWORK_RESULT_OK;
}

//-------------------------------------------------------------------------------------
bool network_create_listening_socket(int32_t* sd, uint32_t port, uint8_t backlog) {
	struct sockaddr_in sServerAddress;
	int32_t _sd;
	int32_t result;

	// open a socket for ftp data listen
	*sd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	_sd = *sd;

	if (_sd > 0) {
		// enable non-blocking mode
		uint32_t option = fcntl(_sd, F_GETFL, 0);
		option |= O_NONBLOCK;
		fcntl(_sd, F_SETFL, option);

		// enable address reusing
		option = 1;
		result = setsockopt(_sd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

		// bind the socket to a port number
		sServerAddress.sin_family = AF_INET;
		sServerAddress.sin_addr.s_addr = INADDR_ANY;
		sServerAddress.sin_len = sizeof(sServerAddress);
		sServerAddress.sin_port = htons(port);

		result |= bind(_sd, (const struct sockaddr*)&sServerAddress, sizeof(sServerAddress));

		// start listening
		result |= listen(_sd, backlog);

		if (!result) {
			return true;
		}
		closesocket(*sd);
	}
	return false;
}

void network_close_socket(int32_t* sd)
{
	if (*sd < 0)
	{
		return;
	}
	closesocket(*sd);
	*sd = -1;
}

#endif