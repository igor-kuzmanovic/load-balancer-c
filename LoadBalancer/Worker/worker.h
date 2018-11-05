#pragma once

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define SERVER_IP		"127.0.0.1"
#define SERVER_PORT		5060

#define BUFFER_SIZE		1000000
#define WR_BUFFER_SIZE	BUFFER_SIZE + sizeof(void *)

#define PRINT_REQUEST	0

#define SLEEP_TIME		2000

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <conio.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "../Common/tcp_helpers.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")