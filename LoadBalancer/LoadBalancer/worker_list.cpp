#include "load_balancer.h"

p_WorkerList worker_list_create() {
	/* Allocates the memory for the worker list structure and initializes it */
	p_WorkerList list = (p_WorkerList)malloc(sizeof(WorkerList));
	list->head = NULL;
	list->tail = NULL;
	list->count = 0;
	list->signalFull = CreateSemaphore(NULL, 0, WORKER_CAPACITY, NULL);
	list->signalEmpty = CreateSemaphore(NULL, WORKER_CAPACITY, WORKER_CAPACITY, NULL);
	InitializeCriticalSection(&(list->lock));
	return list;
}

void worker_list_destroy(p_WorkerList list) {
	/* Closes the semaphore handle, destroys the lock and frees the allocated memory */
	EnterCriticalSection(&(list->lock));

	p_Worker current = list->head;
	while (list->head != NULL) {
		current = list->head->next;
		free(list->head);
		list->head = current;
	}
	list->tail = NULL;
	CloseHandle(list->signalEmpty);
	CloseHandle(list->signalFull);

	LeaveCriticalSection(&(list->lock));
	DeleteCriticalSection(&(list->lock));

	free(list);
	list = NULL;
}

void worker_list_push(p_WorkerList list, p_Worker worker) {
	/* Pushes the worker into the list */
	WaitForSingleObject(list->signalEmpty, INFINITE);
	EnterCriticalSection(&(list->lock));

	worker->next = NULL;
	if (list->head == NULL) {
		list->head = worker;
		list->tail = worker;
	}
	else {
		list->tail->next = worker;
		list->tail = list->tail->next;
	}
	list->count++;

	LeaveCriticalSection(&(list->lock));
	ReleaseSemaphore(list->signalFull, 1, NULL);
}

p_Worker worker_list_pop(p_WorkerList list) {
	/* Waits for the signal that there is a worker waiting and pops it */
	WaitForSingleObject(list->signalFull, INFINITE);
	EnterCriticalSection(&(list->lock));

	p_Worker worker = NULL;
	worker = list->head;
	list->head = list->head->next;
	list->count--;

	LeaveCriticalSection(&(list->lock));
	ReleaseSemaphore(list->signalEmpty, 1, NULL);

	return worker;
}

p_Worker worker_list_get(p_WorkerList list, const SOCKET socket) {
	/* Waits for the signal that there is a worker waiting and gets it */
	WaitForSingleObject(list->signalFull, INFINITE);
	EnterCriticalSection(&(list->lock));

	if (list->head == NULL) {
		LeaveCriticalSection(&(list->lock));
		ReleaseSemaphore(list->signalFull, 1, NULL);

		return NULL;
	}
	p_Worker worker = list->head;
	p_Worker previous = NULL;
	while (worker->socket != socket) {
		if (worker->next == NULL) {
			LeaveCriticalSection(&(list->lock));
			ReleaseSemaphore(list->signalFull, 1, NULL);

			return NULL;
		}
		previous = worker;
		worker = worker->next;
	}
	if (worker == list->head) {
		list->head = list->head->next;
	}
	else {
		previous->next = worker->next;
	}
	list->count--;

	LeaveCriticalSection(&(list->lock));
	ReleaseSemaphore(list->signalEmpty, 1, NULL);

	return worker;
}

void worker_list_print(p_WorkerList list) {
	/* Prints out all the contained sockets */
	EnterCriticalSection(&(list->lock));

	p_Worker current = list->head;
	printf("Worker List: ");
	while (current != NULL) {
		printf("[ %d ]", (int)current->socket);
		current = current->next;
	}
	printf("\n");

	LeaveCriticalSection(&(list->lock));
}