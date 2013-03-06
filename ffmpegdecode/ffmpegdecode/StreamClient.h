#pragma once

#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <queue>
using namespace std;
#define RECVBUFSIZE 65535
#define MAXQUEUENUM 25
#define DEFAULT_PORT 27015


void cleanUpStreamClients();
bool recvData(pair<char*,int> &data);
bool setupStreamClient();
void setLocalPort(int port);

