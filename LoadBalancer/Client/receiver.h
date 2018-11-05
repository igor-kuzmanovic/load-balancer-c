#pragma once

#include "client.h"

/*
 * Function: start_receiver
 * ------------------------
 * Receives the responses from the server and prints out the whole message or the message length,
 * determined by PRINT_RESPONSE
 *
 * lpParam (type of SOCKET): the socket from which the responses are received
 *
 * returns: zero
 */
DWORD WINAPI start_receiver(LPVOID lpParam);