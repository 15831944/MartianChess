#ifndef _MC_THREAD_H
#define _MC_THREAD_H

#include "stdafx.h"
#include <winsock2.h>
#include "MCRules.h"

struct ThreadData
{
	int port;
	char *servername;
};

void LaunchClientThread(ThreadData data);
void SendToServer(char *msg);

void ClientThread(LPVOID param);

#endif