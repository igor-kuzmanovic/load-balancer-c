#pragma once

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define SERVER_IP		"127.0.0.1"	/* Ip address of the target server */
#define SERVER_PORT		5059		/* TCP port of the target server */

#define BUFFER_SIZE		1000000		/* Size of send and receive buffers */

#define PRINT_RESPONSE	0			/* 1 - Prints out the response
									   0 - Prints out only the size of the response */

#define NO_OF_REQUESTS	20			/* Number of requests to send */

#define SLEEP_TIME		500			/* Sleep time between sending requests (milliseconds) */

/* System libraries */
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <conio.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

/* User-made libraries */
#include "../Common/tcp_helpers.h"	/* A library of helpful TCP functions */
#include "sender.h"
#include "receiver.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")