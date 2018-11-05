#include "load_balancer.h"

int main() {
	p_WorkerList freeWRList = worker_list_create();		/* Creates a pointer for a list of free workers */
	p_WorkerList busyWRList = worker_list_create();		/* Creates a pointer for a list of busy workers */
	p_RequestQueue queue = request_queue_create();		/* Creates a pointer for a request queue */

	/* Starts the 'Worker Receiver - Dispatcher' component in a new thread */
	WorkerReceiverDispatcherData workerReceiverDispatcherData;
	workerReceiverDispatcherData.queue = queue;
	workerReceiverDispatcherData.freeWRList = freeWRList;
	workerReceiverDispatcherData.busyWRList = busyWRList;
	HANDLE threadWorkerReceiver = CreateThread(NULL, 0, &start_worker_receiver_dispatcher, &workerReceiverDispatcherData, NULL, NULL);
	if (threadWorkerReceiver == NULL) {
		printf("Function 'CreateThread' for 'Worker Receiver Thread' failed with error: %d.\n", GetLastError());
		request_queue_destroy(queue);
		worker_list_destroy(busyWRList);
		worker_list_destroy(freeWRList);
		return 0;
	}

	/* Starts the 'Manager' component in a new thread */
	ManagerData managerData;
	managerData.queue = queue;
	managerData.freeWRList = freeWRList;
	managerData.busyWRList = busyWRList;
	HANDLE threadManager = CreateThread(NULL, 0, &start_manager, &managerData, NULL, NULL);
	if (threadManager == NULL) {
		printf("Function 'CreateThread' for 'Manager Thread' failed with error: %d.\n", GetLastError());
		CloseHandle(threadWorkerReceiver);
		request_queue_destroy(queue);
		worker_list_destroy(busyWRList);
		worker_list_destroy(freeWRList);
		return 0;
	}

	/* Starts the 'Client Receiver - Dispatcher' component in a new thread */
	ClientReceiverDispatcherData clientReceiverDispatcherData;
	clientReceiverDispatcherData.queue = queue;
	HANDLE threadClientReceiverDispatcher = CreateThread(NULL, 0, &start_client_receiver_dispatcher, &clientReceiverDispatcherData, NULL, NULL);
	if (threadClientReceiverDispatcher == NULL) {
		printf("Function 'CreateThread' for 'Client Receiver Dispatcher Thread' failed with error: %d.\n", GetLastError());
		CloseHandle(threadManager);
		CloseHandle(threadWorkerReceiver);
		request_queue_destroy(queue);
		worker_list_destroy(busyWRList);
		worker_list_destroy(freeWRList);
		return 0;
	}

	getchar();		/* Shuts down the program when a key is pressed */

	/* Closes the thread handles and destroys the worker lists and the request queue */
	CloseHandle(threadClientReceiverDispatcher);
	CloseHandle(threadManager);
	CloseHandle(threadWorkerReceiver);
	request_queue_destroy(queue);
	worker_list_destroy(busyWRList);
	worker_list_destroy(freeWRList);
	return 0;
}