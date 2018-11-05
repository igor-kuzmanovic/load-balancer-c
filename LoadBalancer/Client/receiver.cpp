#include "client.h"

DWORD WINAPI start_receiver(LPVOID lpParam) {
	SOCKET socket = *(SOCKET *)lpParam;

	int result = 0;

	char *dataBuffer = (char *)malloc(BUFFER_SIZE + sizeof(char));
	for (int i = 0; i < NO_OF_REQUESTS; i++) {
		/* Receives the responses from the server */
		result = receive(socket, dataBuffer);
		if (result > 0) {
			/* If print response is set to 1 print the message, otherwise print the number of received bytes */
			if (PRINT_RESPONSE) {
				dataBuffer[BUFFER_SIZE] = '\0';
				printf("Server %d sent: %s.\n", (int)socket, dataBuffer);
			}
			else {
				printf("Server %d sent %d bytes.\n", (int)socket, result);
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
	}

	free(dataBuffer);
	return 0;
}