#pragma once

#include "load_balancer.h"

/* A structure that represents a single packed request from the client and data about his response queue */
typedef struct request_t {
	int dataLength;			/* The length of the data */
	char *data;				/* A pointer to the allocated memory that holds the data */
} Request;

typedef request_t *p_Request;

/* A structure that hold all the needed data to manipulate a circular request queue */
typedef struct request_queue_t {
	int tail;									/* An index of the first element in the queue */
	int head;									/* An index of the last element in the queue */
	int count;									/* Number of requests in the queue */
	Request requests[REQUEST_QUEUE_CAPACITY];	/* An array of requests, capacity is determined by REQUEST_QUEUE_CAPACITY */
	HANDLE signalFull;							/* A pointer to the semaphore that prevents popping from an empty queue */
	HANDLE signalEmpty;							/* A pointer to the semaphore that prevents push to a full queue */
	CRITICAL_SECTION lock;						/* A critical section used in request queue functions to prevent concurrency errors */
} RequestQueue;

typedef struct request_queue_t *p_RequestQueue;

p_RequestQueue request_queue_create();
void request_queue_destroy(p_RequestQueue queue);
void request_queue_push(p_RequestQueue queue, const Request request);
Request request_queue_pop(p_RequestQueue queue);
void request_queue_print(p_RequestQueue queue);