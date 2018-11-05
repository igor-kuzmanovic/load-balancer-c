#include "client.h"

DWORD WINAPI start_sender(LPVOID lpParam) {
	SOCKET socket = *(SOCKET *)lpParam;

	int result = 0;

	char *dataBuffer = (char *)malloc(BUFFER_SIZE);
	memset(dataBuffer, 'A', BUFFER_SIZE);
	for (int i = 0; i < NO_OF_REQUESTS; i++) {
		/* Periodically sends a request to the server */
		Sleep(SLEEP_TIME);

		result = send(socket, dataBuffer, BUFFER_SIZE);
		if (result > 0) {
			printf("Message successfully sent. Total bytes: %d\n", result);
		}
		else if (result == 0 || WSAGetLastError() == WSAECONNRESET || WSAGetLastError() == WSAECONNABORTED) {
			printf("Connection with server closed.\n");
			break;
		}
		else {
			printf("Function 'send' failed with error: %d.\n", WSAGetLastError());
			break;
		}
	}

	free(dataBuffer);
	return 0;
}