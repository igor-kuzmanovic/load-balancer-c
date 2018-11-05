#pragma once

#include "client.h"

/*
 * Function: start_sender
 * ----------------------
 * Periodically sends the requests to the server
 * Period is defined by SLEEP_TIME
 * Number of requests is defined by NO_OF_REQUESTS
 *
 * lpParam (type of SOCKET): the socket from which the requests are sent
 *
 * returns: zero
 */
DWORD WINAPI start_sender(LPVOID lpParam);