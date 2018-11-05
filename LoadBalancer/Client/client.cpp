#include "client.h"

int main() {
	int result = 0;

	/* Initializes the use of winsock */
	WSADATA wsaData;
	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != NO_ERROR) {
		printf("Function 'WSAStartup' failed with error: %d.\n", WSAGetLastError());
		return 0;
	}

	/* Creates a connect socket to the server */
	SOCKET connectSocket = create_connect_socket(SERVER_IP, SERVER_PORT);
	if (connectSocket == NULL) {
		printf("Failed to create a connect socket.\n");
		WSACleanup();
		return 0;
	}

	printf("Client connect socket is ready.\n");

	/* An array to hold the sender and receiver threads */
	HANDLE threads[2];

	/* Starts periodically sending requests to the server in a new thread */
	threads[0] = CreateThread(NULL, 0, &start_sender, &connectSocket, NULL, NULL);
	if (threads[0] == NULL) {
		printf("Function 'CreateThread' for 'Sender Thread' failed with error: %d.\n", GetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 0;
	}

	/* Starts receiving responses from the server in a new thread */
	threads[1] = CreateThread(NULL, 0, &start_receiver, &connectSocket, NULL, NULL);
	if (threads[1] == NULL) {
		printf("Function 'CreateThread' for 'Receiver Thread' failed with error: %d.\n", GetLastError());
		CloseHandle(threads[0]);
		closesocket(connectSocket);
		WSACleanup();
		return 0;
	}

	/* Shuts down the program when all the messages have been processed */
	WaitForMultipleObjects(2, threads, TRUE, INFINITE);

	/* Sends a shutdown signal to the server */
	result = shutdown(connectSocket, SD_BOTH);
	if (result == NULL) {
		printf("Function 'shutdown' for 'Client' failed with error: %d.\n", WSAGetLastError());
	}

	CloseHandle(threads[0]);
	CloseHandle(threads[1]);
	closesocket(connectSocket);
	WSACleanup();
	return 0;
}