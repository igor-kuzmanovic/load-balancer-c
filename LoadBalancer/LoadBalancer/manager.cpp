#include "load_balancer.h"

DWORD WINAPI start_manager(LPVOID lpParam) {
	ManagerData data = *(ManagerData *)lpParam;
	p_RequestQueue queue = data.queue;
	p_WorkerList freeWRList = data.freeWRList;
	p_WorkerList busyWRList = data.busyWRList;

	int result = 0;

	/* Starts the initial worker */
	HANDLE workerHandle = start_worker(WORKER_PATH);
	if (workerHandle == NULL) {
		return 0;
	}

	p_Worker worker;
	/* Checks the request queue and starts/stops a worker periodically */
	while (true) {
		Sleep(SLEEP_TIME);

		/* Checks if the queue is above 70% of the queue capacity and the worker capacity is reached */
		if (queue->count > REQUEST_QUEUE_CAPACITY * 0.7 && (freeWRList->count + busyWRList->count) < WORKER_CAPACITY) {
			/* Starts another worker */
			HANDLE workerHandle = start_worker(WORKER_PATH);
			if (workerHandle == NULL) {
				break;
			}
		}
		/* Checks if the queue is below 30% of the queue capacity */
		else if (queue->count < REQUEST_QUEUE_CAPACITY * 0.3 && freeWRList->count > 1) {
			/* Pops a free worker from the list and sends a shutdown signal */
			worker = worker_list_pop(freeWRList);
			result = shutdown(worker->socket, SD_BOTH);
			if (result == SOCKET_ERROR) {
				printf("Shutdown failed with error: %d\n", WSAGetLastError());
			}
			free(worker);
		}
	}

	/* Pops the initial worker from the busy workers list and shuts it down */
	worker = worker_list_pop(freeWRList);
	result = shutdown(worker->socket, SD_BOTH);
	if (result == SOCKET_ERROR) {
		printf("Shutdown failed with error: %d\n", WSAGetLastError());
	}
	free(worker);

	return 0;
}

HANDLE start_worker(const LPCSTR path) {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	/* Creates a new process in a separate console */
	CreateProcess(path, NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
	if (pi.hProcess == NULL) {
		printf("Function 'CreateProcess' for 'Start Worker' failed with error: %d.\n", GetLastError());
	}

	return pi.hProcess;
}