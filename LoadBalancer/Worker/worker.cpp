#include "worker.h"

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

	printf("Worker connect socket is ready.\n");

	char *dataBuffer = (char *)malloc(WR_BUFFER_SIZE + sizeof(char));
	while (true) {
		/* Waits for the request from the server */
		result = receive(connectSocket, dataBuffer);
		if (result > 0) {
			if (PRINT_REQUEST) {
				dataBuffer[WR_BUFFER_SIZE] = '\0';
				printf("Server %d sent: %s.\n", (int)connectSocket, dataBuffer);
			}
			else {
				printf("Server %d sent %d bytes.\n", (int)connectSocket, result);
			}
		}
		else if (result == 0 || WSAGetLastError() == WSAECONNRESET) {
			printf("Connection with server closed.\n");
			break;
		}
		else {
			printf("Function 'recv' failed with error: %d.\n", WSAGetLastError());
			break;
		}

		/* Simulates processing time */
		Sleep(SLEEP_TIME);

		/* Sends the response back to the server without modifying the first 4 bytes (8 bytes for x64) */
		memset(dataBuffer + sizeof(void *), 'B', BUFFER_SIZE);
		result = send(connectSocket, dataBuffer, WR_BUFFER_SIZE);
		if (result > 0) {
			printf("Message successfully sent. Total bytes: %d\n", result);
		}
		else if (result == 0 || WSAGetLastError() == WSAECONNRESET) {
			printf("Connection with server closed.\n");
			break;
		}
		else {
			printf("Function 'send' failed with error: %d.\n", WSAGetLastError());
			break;
		}
	}

	free(dataBuffer);
	closesocket(connectSocket);
	WSACleanup();
	return 0;
}
