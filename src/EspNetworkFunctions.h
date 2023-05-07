// Network wrapping primitives, currently used on ESP32, but
// should be adapted if needed on other faster MCUs

#pragma once
#include <ConfigurableFirmata.h>

#define MAX_ACTIVE_INTERFACES   3
#ifdef ESP32
typedef enum {
    E_NETWORK_RESULT_OK = 0,
    E_NETWORK_RESULT_CONTINUE,
    E_NETWORK_RESULT_FAILED
} network_result_t;

bool network_create_listening_socket(int32_t* sd, uint32_t port, uint8_t backlog);
void network_close_socket(int32_t* sd);
network_result_t network_wait_for_connection(int32_t listeningSocket, int32_t* connectionSocket, uint32_t* ip_addr, bool nonblocking);
network_result_t network_recv_non_blocking(int32_t sd, char* buff, int32_t maxlen, int32_t* rxLen);
int network_poll(int32_t* socket);

network_result_t network_send(int32_t socket, byte b);
network_result_t network_send(int32_t socket, byte b, bool isLast);
network_result_t network_send(int32_t socket, const byte* data, size_t length);

#endif