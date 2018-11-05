#pragma once

#include "load_balancer.h"

/* A structure intended for use with the client receiver - dispatcher component */
typedef struct client_receiver_dispatcher_data_t {
	p_RequestQueue queue;			/* A pointer to the request queue */
} ClientReceiverDispatcherData;

/* A structure intended for use with the client receive thread */
typedef struct client_receive_data_t {
	SOCKET socket;					/* A pointer to the TCP socket of the client */
	p_RequestQueue requestQueue;	/* A pointer to the request queue */
	p_ResponseQueue responseQueue;	/* A pointer to the client's unique response queue */
} ClientReceiveData;

/* A structure intended for use with the client dispatch thread */
typedef struct client_dispatch_data_t {
	SOCKET socket;					/* A pointer to the TCP socket of the client */
	p_ResponseQueue queue;			/* A pointer to the client's unique response queue */
} ClientDispatchData;

/*
 * Function: start_client_receiver_dispatcher
 * ------------------------------------------
 * Starts a component that creates a listen socket for clients
 * Creates accept sockets and separate threads for each client
 *
 * lpParam (type of ClientReceiverDispatcherData)
 *
 * returns: ZERO
 */
DWORD WINAPI start_client_receiver_dispatcher(LPVOID lpParam);

/*
 * Function: start_client_receive
 * ------------------------------
 * Receives requests from the client, packs them and pushes them to the request queue
 *
 * lpParam (type of ClientReceiveData)
 *
 * returns: ZERO
 */
DWORD WINAPI start_client_receive(LPVOID lpParam);

/*
 * Function: start_client_dispatch
 * -------------------------------
 * Reads responses from the clients personal queue and sends them back to the client
 *
 * lpParam (type of ClientDispatchData)
 *
 * returns: ZERO
 */
DWORD WINAPI start_client_dispatch(LPVOID lpParam);