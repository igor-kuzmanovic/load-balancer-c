#pragma once

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

/* Timeout for the timeval structure used in select function */
#define SELECT_TIMEOUT_SECONDS		1
#define SELECT_TIMEOUT_MICROSECONDS	0

/* System libraries */
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <conio.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

/*
 * Function: create_listen_socket
 * ------------------------------
 * Creates a non-blocking TCP listen socket for the specified port
 *
 * port: the port on which to bind the TCP socket
 *
 * returns: a valid listen socket on success
 *          INVALID_SOCKET if an error occurs
 */
SOCKET create_listen_socket(const u_short port);

/*
 * Function: create_connect_socket
 * -------------------------------
 * Creates a non-blocking TCP connect socket to the specified ip address and port
 *
 * ipAddress: the ip address of the server
 * port: the port of the server
 *
 * returns: a valid connect socket on success
 *          INVALID_SOCKET if an error occurs
 */
SOCKET create_connect_socket(const char *ipAddress, const u_short port);

/*
 * Function: select_for_receive
 * ----------------------------
 * Puts the specified socket into a read set and periodically calls the select function in the read mode
 * until there is a message to be read from the socket
 * The timeout period is defined by SELECT_TIMEOUT_SECONDS/MICROSECONDS
 *
 * socket: the socket upon which is the select function called
 *
 * returns: one if there is a message ready to be read
 *          SOCKET_ERROR if an error occurs
 */
int select_for_receive(SOCKET socket);

/*
 * Function: select_for_send
 * -------------------------
 * Puts the specified socket into a write set and periodically calls the select function in the write mode
 * until there is a message to be sent from the socket
 * The timeout period is defined by SELECT_TIMEOUT_SECONDS/MICROSECONDS
 *
 * socket: the socket upon which is the select function called
 *
 * returns: one if there is a message ready to be sent
 *          SOCKET_ERROR if an error occurs
 */
int select_for_send(SOCKET socket);

/*
 * Function: receive
 * -----------------
 * Gets the first 4 bytes from the specified socket that determine the length of the remaining data
 * Gets the rest of the message into the specified buffer
 *
 * socket: the socket from which to receive the message
 * buffer: the buffer which is used to store the message
 *
 * returns: the result of the winsock recv function
 */
int receive(SOCKET socket, char *buffer);

/*
 * Function: send
 * --------------
 * Determines the length of the string in the specified buffer
 * and sends that string using the specified socket
 * WARNING: The data contained must contain a null terminator!
 * 
 * socket: the socket on which to send the message
 * buffer: the buffer that contains the string that needs to be sent
 *
 * returns: the result of the winsock send function
 */
int send(SOCKET socket, const char *buffer);

/*
 * Function: send
 * --------------
 * Sends the data from the specified buffer to the specified socket
 * The length of the data sent is determined by the specified length
 *
 * socket: the socket on which to send the message
 * buffer: the buffer that contains the message that needs to be sent
 * length: the length of the message to be sent
 *
 * returns: the result of the winsock send function
 */
int send(SOCKET socket, const char *buffer, int length);