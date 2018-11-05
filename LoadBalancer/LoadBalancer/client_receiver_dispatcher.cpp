#include "load_balancer.h"

DWORD WINAPI start_client_receiver_dispatcher(LPVOID lpParam) {
	ClientReceiverDispatcherData data = *(ClientReceiverDispatcherData *)lpParam;
	p_RequestQueue queue = data.queue;

	int result = 0;

	/* Initializes the use of winsock */
	WSADATA wsaData;
	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != NO_ERROR) {
		printf("Function 'WSAStartup' for 'Client Receiver - Dispatcher' failed with error: %d.\n", WSAGetLastError());
		return 0;
	}

	/* Creates a listen socket for clients */
	SOCKET listenSocket = create_listen_socket(CLIENT_PORT);
	if (listenSocket == NULL) {
		printf("Failed to create a listen socket for 'Client Receiver - Dispatcher'.\n");
		WSACleanup();
		return 0;
	}

	printf("Client receiver socket is set to listening mode. Waiting for new client connections...\n");

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
			printf("Function 'select' for 'Client Receiver - Dispatcher' failed with error: %d.\n", WSAGetLastError());
			break;
		}
		else {
			/* Creates an accept socket for the new client */
			sockaddr_in clientAddr;
			int clientAddrSize = sizeof(struct sockaddr_in);
			SOCKET acceptSocket = accept(listenSocket, (struct sockaddr *)&clientAddr, &clientAddrSize);
			if (acceptSocket == INVALID_SOCKET) {
				printf("Function 'accept' for 'Client Receiver - Dispatcher' failed with error: %d.\n", WSAGetLastError());
				break;
			}

			/* Puts the accept socket in the non-blocking mode */
			u_long mode = 1;
			result = ioctlsocket(acceptSocket, FIONBIO, &mode);
			if (result != NO_ERROR) {
				printf("Function 'ioctlsocket' for 'Client Receiver - Dispatcher' failed with error: %d.\n", WSAGetLastError());
				closesocket(acceptSocket);
				break;
			}

			/* Creates a response queue for the client */
			p_ResponseQueue responseQueue = response_queue_create();

			/* Starts receiving requests from the client in a separate thread */
			ClientReceiveData clientReceiveData;
			clientReceiveData.socket = acceptSocket;
			clientReceiveData.requestQueue = queue;
			clientReceiveData.responseQueue = responseQueue;
			HANDLE threadClientReceive = CreateThread(NULL, 0, &start_client_receive, &clientReceiveData, NULL, 0);
			if (threadClientReceive == NULL) {
				printf("Function 'CreateThread' for 'Client Receiver - Dispatcher' failed with error: %d.\n", GetLastError());
				response_queue_destroy(responseQueue);
				closesocket(acceptSocket);
				break;
			}

			/* Starts sending responses to the client in a separate thread */
			ClientDispatchData clientDispatchData;
			clientDispatchData.socket = acceptSocket;
			clientDispatchData.queue = responseQueue;
			HANDLE threadClientDispatcher = CreateThread(NULL, 0, &start_client_dispatch, &clientDispatchData, NULL, 0);
			if (threadClientDispatcher == NULL) {
				printf("Function 'CreateThread' for 'Client Receiver - Dispatcher' failed with error: %d.\n", GetLastError());
				response_queue_destroy(responseQueue);
				CloseHandle(threadClientReceive);
				closesocket(acceptSocket);
				break;
			}

			printf("New client accepted. Client address: %s:%d.\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
		}
	}

	closesocket(listenSocket);
	WSACleanup();
	return 0;
}

DWORD WINAPI start_client_receive(LPVOID lpParam) {
	ClientReceiveData data = *(ClientReceiveData *)lpParam;
	SOCKET socket = data.socket;
	p_RequestQueue requestQueue = data.requestQueue;
	p_ResponseQueue responseQueue = data.responseQueue;

	int result = 0;

	Request request;	/* A structure for packing client requests */
	while (true) {
		char *dataBuffer = (char *)malloc(WR_BUFFER_SIZE);
		result = receive(socket, dataBuffer + sizeof(p_ResponseQueue));
		if (result > 0) {
			/* Packs the client's response queue address to the start of the buffer */
			*(p_ResponseQueue *)dataBuffer = responseQueue;
			/* Packs the length of the received request */
			request.dataLength = result;
			/* Packs the address of the used data buffer */
			request.data = dataBuffer;
			printf("Client %d sent %d bytes.\n", (int)socket, result);
			/* Pushes the request into the request queue */
			request_queue_push(requestQueue, request);
			/* Increment the number of expected responses */
			responseQueue->expectedCount++;
		}
		else if (result == 0 || WSAGetLastError() == WSAECONNRESET) {
			printf("Connection with client closed.\n");
			free(dataBuffer);
			break;
		}
		else {
			printf("Function 'recv' for 'Client Receive' failed with error: %d.\n", WSAGetLastError());
			free(dataBuffer);
			break;
		}
	}

	closesocket(socket);
	return 0;
}

DWORD WINAPI start_client_dispatch(LPVOID lpParam) {
	ClientDispatchData data = *(ClientDispatchData *)lpParam;
	SOCKET socket = data.socket;
	p_ResponseQueue queue = data.queue;

	int result = 0;

	Response response;	/* A structure for packing responses */
	while (true) {
		/* Pops a response from the queue */
		response = response_queue_pop(queue);
		/* Sends the data to the client but skips the first 4 bytes (8 in the x64 build) */
		result = send(socket, response.data + sizeof(p_ResponseQueue), response.dataLength - sizeof(p_ResponseQueue));
		/* Frees the allocated memory */
		free(response.data);
		/* Decrements the number of expected responses */
		queue->expectedCount--;
		if (result > 0) {
			printf("Response successfully sent to client. Total bytes: %d\n", result);
		}
		else if (result == 0 || WSAGetLastError() == WSAECONNRESET || WSAGetLastError() == WSAENOTSOCK) {
			printf("Connection with client closed.\n");
			break;
		}
		else {
			printf("Function 'send' for 'Client Dispatch' failed with error: %d.\n", WSAGetLastError());
			break;
		}
	}

	/* If the client shuts down the connection all the responses must be popped and their data memory freed */
	while (queue->expectedCount > 0) {
		response = response_queue_pop(queue);
		free(response.data);
		queue->expectedCount--;
	}

	response_queue_destroy(queue);	/* Destroys the client's response queue */
	closesocket(socket);
	return 0;
}