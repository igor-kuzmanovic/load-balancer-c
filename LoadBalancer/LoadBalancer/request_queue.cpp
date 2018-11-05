#include "load_balancer.h"

p_RequestQueue request_queue_create() {
	/* Allocates the memory for the request queue structure and initializes it */
	p_RequestQueue queue = (p_RequestQueue)malloc(sizeof(RequestQueue));
	queue->head = -1;
	queue->tail = -1;
	queue->count = 0;
	queue->signalFull = CreateSemaphore(NULL, 0, REQUEST_QUEUE_CAPACITY, NULL);
	queue->signalEmpty = CreateSemaphore(NULL, REQUEST_QUEUE_CAPACITY, REQUEST_QUEUE_CAPACITY, NULL);
	InitializeCriticalSection(&(queue->lock));
	return queue;
}

void request_queue_destroy(p_RequestQueue queue) {
	/* Closes the semaphore handles, destroys the lock and frees the allocated memory */
	EnterCriticalSection(&(queue->lock));

	CloseHandle(queue->signalEmpty);
	CloseHandle(queue->signalFull);

	LeaveCriticalSection(&(queue->lock));
	DeleteCriticalSection(&(queue->lock));

	free(queue);
	queue = NULL;
}

void request_queue_push(p_RequestQueue queue, const Request request) {
	/* Waits for the signal that there is a free location and pushes the new request into the queue */
	WaitForSingleObject(queue->signalEmpty, INFINITE);
	EnterCriticalSection(&(queue->lock));

	if (queue->tail == -1)
		queue->head = 0;
	queue->tail = (queue->tail + 1) % REQUEST_QUEUE_CAPACITY;
	queue->requests[queue->tail] = request;
	queue->count++;

	LeaveCriticalSection(&(queue->lock));
	ReleaseSemaphore(queue->signalFull, 1, NULL);
}

Request request_queue_pop(p_RequestQueue queue) {
	/* Waits for the signal that there is a request waiting in the queue and pops it */
	WaitForSingleObject(queue->signalFull, INFINITE);
	EnterCriticalSection(&(queue->lock));

	Request request;
	int current = queue->head;
	queue->head = (queue->head + 1) % REQUEST_QUEUE_CAPACITY;
	request.dataLength = queue->requests[current].dataLength;
	request.data = queue->requests[current].data;
	queue->count--;

	LeaveCriticalSection(&(queue->lock));
	ReleaseSemaphore(queue->signalEmpty, 1, NULL);

	return request;
}

void request_queue_print(p_RequestQueue queue) {
	/* Prints out the whole queue */
	EnterCriticalSection(&(queue->lock));

	printf("Request queue: ");
	for (int i = 0; i < REQUEST_QUEUE_CAPACITY; i++)
		printf("[ %d ]", (int)queue->requests[i].dataLength);
	printf("\n");

	LeaveCriticalSection(&(queue->lock));
}
