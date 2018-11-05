#pragma once

#include "load_balancer.h"

/* A structure intended for use with the manager component */
typedef struct manager_data_t {
	p_RequestQueue queue;		/* A pointer to the request queue */
	p_WorkerList freeWRList;	/* A pointer to the list of free workers */
	p_WorkerList busyWRList;	/* A pointer to the list of busy workers */
} ManagerData;

/*
 * Function: start_manager
 * -----------------------
 * Starts a component that starts/stops workers depending on the state of the request queue
 *
 * lpParam (type of ManagerData)
 *
 * returns: ZERO
 */
DWORD WINAPI start_manager(LPVOID lpParam);

/*
 * Function: start_worker
 * ----------------------
 * Starts a worker process in a new console and returns a process handle for it
 *
 * path: A path to the worker executable
 *
 * returns: On success returns a handle of the created process
 *		    On failure returns NULL
 */
HANDLE start_worker(const LPCSTR path);