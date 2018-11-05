#include "load_balancer.h"

p_ResponseQueue response_queue_create() {
	/* Allocates the memory for the response queue structure and initializes it */
	p_ResponseQueue queue = (p_ResponseQueue)malloc(sizeof(RequestQueue));
	queue->head = -1;
	queue->tail = -1;
	queue->expectedCount = 0;
	queue->signalFull = CreateSemaphore(NULL, 0, RESPONSE_QUEUE_CAPACITY, NULL);
	queue->signalEmpty = CreateSemaphore(NULL, RESPONSE_QUEUE_CAPACITY, RESPONSE_QUEUE_CAPACITY, NULL);
	InitializeCriticalSection(&(queue->lock));
	return queue;
}

void response_queue_destroy(p_ResponseQueue queue) {
	/* Closes the semaphore handles, destroys the lock and frees the allocated memory */
	EnterCriticalSection(&(queue->lock));

	CloseHandle(queue->signalEmpty);
	CloseHandle(queue->signalFull);

	LeaveCriticalSection(&(queue->lock));
	DeleteCriticalSection(&(queue->lock));

	free(queue);
	queue = NULL;
}

void response_queue_push(p_ResponseQueue queue, const Response response) {
	/* Waits for the signal that there is a free location and pushes the new response into the queue */
	WaitForSingleObject(queue->signalEmpty, INFINITE);
	EnterCriticalSection(&(queue->lock));

	if (queue->tail == -1)
		queue->head = 0;
	queue->tail = (queue->tail + 1) % RESPONSE_QUEUE_CAPACITY;
	queue->responses[queue->tail] = response;

	LeaveCriticalSection(&(queue->lock));
	ReleaseSemaphore(queue->signalFull, 1, NULL);
}

Response response_queue_pop(p_ResponseQueue queue) {
	/* Waits for the signal that there is a response waiting in the queue and pops it */
	WaitForSingleObject(queue->signalFull, INFINITE);
	EnterCriticalSection(&(queue->lock));

	Response response;
	int current = queue->head;
	queue->head = (queue->head + 1) % RESPONSE_QUEUE_CAPACITY;
	response.dataLength = queue->responses[current].dataLength;
	response.data = queue->responses[current].data;

	LeaveCriticalSection(&(queue->lock));
	ReleaseSemaphore(queue->signalEmpty, 1, NULL);

	return response;
}

void response_queue_print(p_ResponseQueue queue) {
	/* Prints out the whole queue */
	EnterCriticalSection(&(queue->lock));

	printf("Response queue: ");
	for (int i = 0; i < RESPONSE_QUEUE_CAPACITY; i++)
		printf("[ %d ]", (int)queue->responses[i].dataLength);
	printf("\n");

	LeaveCriticalSection(&(queue->lock));
}