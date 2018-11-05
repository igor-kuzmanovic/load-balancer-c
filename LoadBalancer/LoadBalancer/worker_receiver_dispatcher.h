#pragma once

#include "load_balancer.h"

/* A structure intended for use with the worker receiver - dispatcher component */
typedef struct worker_receiver_dispatcher_data_t {
	p_WorkerList freeWRList;	/* A pointer to the list of free workers */
	p_WorkerList busyWRList;	/* A pointer to the list of busy workers */
	p_RequestQueue queue;		/* A potiner to the request queue */
} WorkerReceiverDispatcherData;

/* A structure inteded for use with the worker receive thread */
typedef struct worker_receive_data {
	SOCKET socket;				/* A pointer to the TCP socket of the worker */
	p_WorkerList freeWRList;	/* A pointer to the list of free workers */
	p_WorkerList busyWRList;	/* A pointer to the list of busy workers */
} WorkerReceiveData;

/* A structure intended for use with the worker dispatch thread */
typedef struct worker_dispatch_data_t {
	p_WorkerList freeWRList;	/* A pointer to the list of free workers */
	p_WorkerList busyWRList;	/* A pointer to the list of busy workers */
	p_RequestQueue queue;		/* A pointer to the request queue */
} WorkerDispatchData;

/*
 * Function: start_worker_receiver_dispatcher
 * ------------------------------------------
 * Starts a component that creates a listen socket for clients
 * Creates accept sockets and separate threads for each client
 *
 * lpParam (type of WorkerReceiverDispatcherData)
 *
 * returns: ZERO
 */
DWORD WINAPI start_worker_receiver_dispatcher(LPVOID lpParam);

/*
 * Function: start_worker_receive
 * ------------------------------
 * Receives responses from the worker, packs them and pushes them to the client's response queue
 *
 * lpParam (type of WorkerReceiveData)
 *
 * returns: ZERO
 */
DWORD WINAPI start_worker_receive(LPVOID lpParam);

/*
 * Function: start_worker_dispatch
 * -------------------------------
 * Reads requests from the request queue and sends them to their each respective workers
 *
 * lpParam (type of WorkerDispatchData)
 *
 * returns: ZERO
 */
DWORD WINAPI start_worker_dispatch(LPVOID lpParam);
