#pragma once

#include "load_balancer.h"

/* A structure that represents a single packed response from the worker */
typedef struct response_t {
	int dataLength;		/* The length of the data */
	char *data;			/* A pointer to the allocated memory that holds the data */
} Response;

typedef response_t *p_Response;

/* A structure that hold all the needed data to manipulate a circular response queue */
typedef struct response_queue_t {
	int tail;										/* An index of the first element in the queue */
	int head;										/* An index of the last element in the queue */
	int expectedCount;								/* Number expected elements to be pushed into the queue */
	Response responses[RESPONSE_QUEUE_CAPACITY];	/* An array of responses, capacity is determined by RESPONSE_QUEUE_CAPACITY */
	HANDLE signalFull;								/* A pointer to the semaphore that prevents popping from an empty queue */
	HANDLE signalEmpty;								/* A pointer to the semaphore that prevents push to a full queue */
	CRITICAL_SECTION lock;							/* A critical section used in response queue functions to prevent concurrency errors */
} ResponseQueue;

typedef struct response_queue_t *p_ResponseQueue;

p_ResponseQueue response_queue_create();
void response_queue_destroy(p_ResponseQueue queue);
void response_queue_push(p_ResponseQueue queue, const Response response);
Response response_queue_pop(p_ResponseQueue queue);
void response_queue_print(p_ResponseQueue queue);
