#pragma once

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define SERVER_IP					"127.0.0.1"						/* Ip address of the server */
#define CLIENT_PORT					5059							/* Public port for clients */
#define WORKER_PORT					5060							/* Internal port for workers */

#define REQUEST_QUEUE_CAPACITY		100								/* The capacity of the request queue */
#define RESPONSE_QUEUE_CAPACITY		100								/* The capacity of the clients' respose queue */

#define BUFFER_SIZE					1000000							/* The size of the data buffer */
#define WR_BUFFER_SIZE				BUFFER_SIZE + sizeof(void *)	/* The size of the extended data buffer (for workers) */

#define WORKER_CAPACITY				10								/* Number of allowed workers */
#define WORKER_PATH					"../Debug/Worker.exe"			/* The executable path of the worker process */

#define SLEEP_TIME					1000							/* Timeout for the manager (in milliseconds) */

/* System libraries */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <conio.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

/* User-made libraries */
#include "../Common/tcp_helpers.h"
#include "response_queue.h"
#include "request_queue.h"
#include "worker_list.h"
#include "manager.h"
#include "worker_receiver_dispatcher.h"
#include "client_receiver_dispatcher.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")