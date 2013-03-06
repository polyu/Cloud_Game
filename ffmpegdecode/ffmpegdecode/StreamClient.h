#pragma once

#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <queue>
using namespace std;
#define RECVBUFSIZE 65535
#define MAXQUEUENUM 25
void cleanUpStreamClients();
bool recvData(pair<char*,int> &data);
bool setupStreamClient(int port);

