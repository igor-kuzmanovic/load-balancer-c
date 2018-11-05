#include "load_balancer.h"

DWORD WINAPI start_worker_receiver_dispatcher(LPVOID lpParam) {
	WorkerReceiverDispatcherData data = *(WorkerReceiverDispatcherData *)lpParam;
	p_RequestQueue queue = data.queue;
	p_WorkerList freeWRList = data.freeWRList;
	p_WorkerList busyWRList = data.busyWRList;

	int result = 0;

	/* Initializes the use of winsock */
	WSADATA wsaData;
	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != NO_ERROR) {
		printf("Function 'WSAStartup' for 'Worker Receiver - Dispatcher' failed with error: %d.\n", WSAGetLastError());
		return 0;
	}

	/* Creates a listen socket for workers */
	SOCKET listenSocket = create_listen_socket(WORKER_PORT);
	if (listenSocket == NULL) {
		printf("Failed to create a listen socket for 'Worker Receiver - Dispatcher'.\n");
		WSACleanup();
		return 0;
	}

	printf("Worker receiver socket is set to listening mode. Waiting for new worker connections...\n");

	fd_set readfds;		/* A set that holds the listen socket */
	timeval timeVal;	/* A time structure for the select function timeout */
	timeVal.tv_sec = SELECT_TIMEOUT_SECONDS;
	timeVal.tv_usec = SELECT_TIMEOUT_MICROSECONDS;

	while (true) {
		/* Empties the set and adds the listen socket to it */
		FD_ZERO(&readfds);
		FD_SET(listenSocket, &readfds);
		result = select(0, &readfds, NULL, NULL, &timeVal);
		if (result == 0) {
			continue;
		}
		else if (result == SOCKET_ERROR) {
			printf("Function 'select' for 'Worker Receiver - Dispatcher' failed with error: %d.\n", WSAGetLastError());
			break;
		}
		else {
			/* Creates an accept socket for the new worker */
			sockaddr_in clientAddr;
			int clientAddrSize = sizeof(struct sockaddr_in);
			SOCKET acceptSocket = accept(listenSocket, (struct sockaddr *)&clientAddr, &clientAddrSize);
			if (acceptSocket == INVALID_SOCKET) {
				printf("Function 'accept' for 'Worker Receiver - Dispatcher' failed with error: %d.\n", WSAGetLastError());
				break;
			}

			/* Puts the accept socket in the non-blocking mode */
			u_long mode = 1;
			result = ioctlsocket(acceptSocket, FIONBIO, &mode);
			if (result != NO_ERROR) {
				printf("Function 'ioctlsocket' for 'Worker Receiver - Dispatcher' failed with error: %d.\n", WSAGetLastError());
				closesocket(acceptSocket);
				break;
			}

			/* Starts receiving requests from the worker in a separate thread */
			WorkerReceiveData workerReceiveData;
			workerReceiveData.socket = acceptSocket;
			workerReceiveData.freeWRList = freeWRList;
			workerReceiveData.busyWRList = busyWRList;
			HANDLE threadWorkerReceive = CreateThread(NULL, 0, &start_worker_receive, &workerReceiveData, NULL, 0);
			if (threadWorkerReceive == NULL) {
				printf("Function 'CreateThread' for 'Worker Receive Thread' failed with error: %d.\n", GetLastError());
				closesocket(acceptSocket);
				break;
			}

			/* Sends the request to the worker in a new thread */
			WorkerDispatchData workerDispatchData;
			workerDispatchData.freeWRList = freeWRList;
			workerDispatchData.busyWRList = busyWRList;
			workerDispatchData.queue = queue;
			HANDLE threadWorkerDispatcher = CreateThread(NULL, 0, &start_worker_dispatch, &workerDispatchData, NULL, NULL);
			if (threadWorkerDispatcher == NULL) {
				printf("Function 'CreateThread' for 'Worker Dispatch Thread' failed with error: %d.\n", GetLastError());
				CloseHandle(threadWorkerReceive);
				closesocket(acceptSocket);
				break;
			}

			printf("New worker accepted. Worker address: %s:%d.\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
		}
	}

	closesocket(listenSocket);
	WSACleanup();
	return 0;
}

DWORD WINAPI start_worker_receive(LPVOID lpParam) {
	WorkerReceiveData data = *(WorkerReceiveData *)lpParam;
	SOCKET acceptSocket = data.socket;
	p_WorkerList freeWRList = data.freeWRList;
	p_WorkerList busyWRList = data.busyWRList;

	int result = 0;

	/* Allocates the memory for a new worker structure */
	p_Worker worker = (p_Worker)malloc(sizeof(Worker));
	/* Sets its accept socket */
	worker->socket = acceptSocket;
	/* Pushes the worker into the free workers list */
	worker_list_push(freeWRList, worker);

	p_ResponseQueue queue;	/* A pointer to a client's response queue */
	Response response;		/* A structure for packing responses */
	while (true) {
		char *dataBuffer = (char *)malloc(WR_BUFFER_SIZE);
		/* Receives a message from the worker */
		result = receive(acceptSocket, dataBuffer);
		if (result > 0) {
			printf("Worker %d sent %d bytes.\n", (int)acceptSocket, result);
			/* Gets the worker from the busy workers list */
			worker = worker_list_get(busyWRList, acceptSocket);
			/* Gets the client's response queue address from the start of the response */
			queue = *(p_ResponseQueue *)dataBuffer;
			/* Packs the length of the received response */
			response.dataLength = result;
			/* Packs the address of the allocated buffer containing the response */
			response.data = dataBuffer;
			/* Pushes the response into the client's response queue */
			response_queue_push(queue, response);
			/* Pushes the worker back into the free workers list */
			worker_list_push(freeWRList, worker);
		}
		else if (result == 0 || WSAGetLastError() == WSAECONNRESET || WSAGetLastError() == WSAESHUTDOWN) {
			printf("Connection with worker closed.\n");
			free(dataBuffer);
			break;
		}
		else {
			printf("Function 'recv' for 'Worker Receive' failed with error: %d.\n", WSAGetLastError());
			free(dataBuffer);
			break;
		}
	}

	closesocket(acceptSocket);
	return 0;
}

DWORD WINAPI start_worker_dispatch(LPVOID lpParam) {
	WorkerDispatchData data = *(WorkerDispatchData *)lpParam;
	p_WorkerList freeWRList = data.freeWRList;
	p_WorkerList busyWRList = data.busyWRList;
	p_RequestQueue queue = data.queue;

	int result = 0;

	p_Worker worker;
	Request request;	/* A structure for holding client requests */
	while (true) {
		/* Pops a request from the request queue */
		request = request_queue_pop(queue);
		/* Pops a free worker from the list of free workers */
		worker = worker_list_pop(freeWRList);
		/* Sends the serialized request to the worker */
		result = send(worker->socket, request.data, request.dataLength);
		/* Frees the request data memory */
		free(request.data);
		/* Pushes the worker into the list of busy workers */
		worker_list_push(busyWRList, worker);
		if (result > 0) {
			printf("Request successfully sent to worker. Total bytes: %d.\n", result);
		}
		else if (result == 0 || WSAGetLastError() == WSAECONNRESET) {
			printf("Connection with worker closed.\n");
			break;
		}
		else {
			printf("Function 'send' for 'Worker Dispatch' failed with error: %d.\n", WSAGetLastError());
			break;
		}
	}

	closesocket(worker->socket);
	return 0;
}
