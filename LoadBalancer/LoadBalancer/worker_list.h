#pragma once

#include "load_balancer.h"

/* A structure that represents a single worker intended for use as a node in the workers list */
typedef struct worker_t {
	SOCKET socket;			/* A pointer to the TCP socket of the worker */
	worker_t *next;			/* A pointer to the next worker node in the list */
} Worker;

typedef worker_t *p_Worker;

/* A structure that holds all the required data for work with a FIFO worker list */
typedef struct worker_list_t {
	p_Worker head;			/* A pointer to the first worker node */
	p_Worker tail;			/* A pointer to the last worker node */
	int count;				/* Number of workers contained in the list */
	HANDLE signalFull;		/* A pointer to the semaphore that prevents popping from an empty list */
	HANDLE signalEmpty;		/* A pointer to the semaphore that prevents pushing to a full list */
	CRITICAL_SECTION lock;	/* A critical section used in worker list functions to prevent concurrency errors */
} WorkerList;

typedef struct worker_list_t *p_WorkerList;

p_WorkerList worker_list_create();
void worker_list_destroy(p_WorkerList list);
void worker_list_push(p_WorkerList list, p_Worker worker);
p_Worker worker_list_pop(p_WorkerList list);
p_Worker worker_list_get(p_WorkerList list, const SOCKET socket);
void worker_list_print(p_WorkerList list);
