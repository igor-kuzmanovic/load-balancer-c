#include "tcp_helpers.h"

SOCKET create_listen_socket(const u_short port) {
	int result = 0;

	/* Creates a socket */
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		printf("Function 'socket' failed with error: %d.\n", WSAGetLastError());
		closesocket(s);
		return INVALID_SOCKET;
	}

	/* Binds it to a specified port and loopback ip address */
	sockaddr_in serverAddress;
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(port);
	result = bind(s, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
	if (result == SOCKET_ERROR) {
		printf("Function 'bind' failed with error: %d.\n", WSAGetLastError());
		closesocket(s);
		return INVALID_SOCKET;
	}

	/* Starts listening */
	result = listen(s, SOMAXCONN);
	if (result == SOCKET_ERROR) {
		printf("Function 'listen' failed with error: %d.\n", WSAGetLastError());
		closesocket(s);
		return INVALID_SOCKET;
	}

	/* Sets the socket into non-blocking mode */
	u_long mode = 1;
	result = ioctlsocket(s, FIONBIO, &mode);
	if (result != NO_ERROR) {
		printf("Function 'ioctlsocket' failed with error: %d.\n", WSAGetLastError());
		closesocket(s);
		return INVALID_SOCKET;
	}

	return s;
}

SOCKET create_connect_socket(const char *ipAddress, const u_short port) {
	int result = 0;

	/* Creates a socket */
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		printf("Function 'socket' failed with error: %d.\n", WSAGetLastError());
		closesocket(s);
		return INVALID_SOCKET;
	}

	/* Connects to the specified ip address and port */
	sockaddr_in serverAddress;
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(ipAddress);
	serverAddress.sin_port = htons(port);
	result = connect(s, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
	if (result == SOCKET_ERROR) {
		printf("Function 'connect' failed with error: %d.\n", WSAGetLastError());
		closesocket(s);
		return INVALID_SOCKET;
	}

	/* Sets the socket into non-blocking mode */
	u_long mode = 1;
	result = ioctlsocket(s, FIONBIO, &mode);
	if (result != NO_ERROR) {
		printf("Function 'ioctlsocket' failed with error: %d.\n", WSAGetLastError());
		closesocket(s);
		return INVALID_SOCKET;
	}

	return s;
}

int select_for_receive(SOCKET socket) {
	int result = 0;

	fd_set readfds;		/* A set that holds the socket */
	timeval timeVal;	/* A time structure for the select function timeout */
	timeVal.tv_sec = SELECT_TIMEOUT_SECONDS;
	timeVal.tv_usec = SELECT_TIMEOUT_MICROSECONDS;

	while (true) {
		/* Empties the set and adds the socket to it */
		FD_ZERO(&readfds);
		FD_SET(socket, &readfds);

		result = select(0, &readfds, NULL, NULL, &timeVal);
		if (result == 0)
			continue;
		else if (result == SOCKET_ERROR)
			return result;
		else
			return 1;
	}
}

int select_for_send(SOCKET socket) {
	int result = 0;

	fd_set writefds;	/* A set that holds the socket */
	timeval timeVal;	/* A time structure for the select function timeout */
	timeVal.tv_sec = SELECT_TIMEOUT_SECONDS;
	timeVal.tv_usec = SELECT_TIMEOUT_MICROSECONDS;

	while (true) {
		/* Empties the set and adds the socket to it */
		FD_ZERO(&writefds);
		FD_SET(socket, &writefds);

		result = select(0, NULL, &writefds, NULL, &timeVal);
		if (result == 0)
			continue;
		else if (result == SOCKET_ERROR)
			return result;
		else
			return 1;
	}
}

int receive(SOCKET socket, char *buffer) {
	int result = 0;

	int bytesReceived = 0;
	/* Calls the recv function until the first 4 bytes are received */
	while (bytesReceived < sizeof(int)) {
		result = select_for_receive(socket);
		if (result < 0)
			return result;

		result = recv(socket, buffer + bytesReceived, sizeof(int) - bytesReceived, 0);
		if (result <= 0)
			return result;

		bytesReceived += result;
	}

	/* Gets the length of the remaining message */
	int length = *(int *)buffer;

	bytesReceived = 0;
	/* Calls the recv function until it receives the whole message */
	while (bytesReceived < length) {
		result = select_for_receive(socket);
		if (result < 0)
			return result;

		result = recv(socket, buffer + bytesReceived, length - bytesReceived, 0);
		if (result <= 0)
			return result;

		bytesReceived += result;
	}

	return bytesReceived;
}

int send(SOCKET socket, const char *buffer) {
	int result = 0;

	/* Gets the length of the message */
	int length = (int)strlen(buffer);
	/* Allocates new memory so the length of the message can be sent before the message */
	char tempBuffer[sizeof(int)];
	/* Packs the length of the message at the start of the buffer */
	*(int *)tempBuffer = length;

	int bytesSent = 0;
	/* Calls the send function until the first 4 bytes are sent */
	while (bytesSent < (int)sizeof(int)) {
		result = select_for_send(socket);
		if (result < 0) {
			return result;
		}

		result = send(socket, tempBuffer + bytesSent, sizeof(int) - bytesSent, 0);
		if (result <= 0) {
			return result;
		}

		bytesSent += result;
	}

	bytesSent = 0;
	/* Calls the send function until the whole message is sent */
	while (bytesSent < length) {
		result = select_for_send(socket);
		if (result < 0)
			return result;

		result = send(socket, buffer + bytesSent, length - bytesSent, 0);
		if (result <= 0)
			return result;

		bytesSent += result;
	}

	return bytesSent;
}

int send(SOCKET socket, const char *buffer, int length) {
	int result = 0;

	/* Allocates new memory so the length of the message can be sent before the message */
	char tempBuffer[sizeof(int)];
	/* Packs the length of the message at the start of the buffer */
	*(int *)tempBuffer = length;

	int bytesSent = 0;
	/* Calls the send function until the first 4 bytes are sent */
	while (bytesSent < (int)sizeof(int)) {
		result = select_for_send(socket);
		if (result < 0)
			return result;

		result = send(socket, tempBuffer + bytesSent, sizeof(int) - bytesSent, 0);
		if (result <= 0)
			return result;

		bytesSent += result;
	}

	bytesSent = 0;
	/* Calls the send function until the whole message is sent */
	while (bytesSent < length) {
		result = select_for_send(socket);
		if (result < 0)
			return result;

		result = send(socket, buffer + bytesSent, length - bytesSent, 0);
		if (result <= 0)
			return result;

		bytesSent += result;
	}

	return bytesSent;
}