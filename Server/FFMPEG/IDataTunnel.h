#pragma once
#include "stdafx.h"
#include <WinSock2.h>
#include <windows.h>
#include <string>
#include <queue>
#include <time.h>
using namespace std;
#define DEFAULT_LOCALPORT 20000
#define NETWORKMTU 1450 
#define MAXWAITQUEUENUM 20
#define MAXPENDINGTIME 1*60*1000
//===========Explation
#define VIDEODATAHEADERTYPE 0x1
#define AUDIODATAHEADERTYPE 0x2
#define CONTROLERDATAHEADERTYPE 0x4
#define CONNECTIONREQUESTHEADERTYPE 0x8
#define LASTMARKERBIT 0x40
#define HEADERLENGTH 1
//0T00 LTTT From left to right, second bit show whether the last frame, From right to left, show the type of packet!
//like Video Audio Controller, L means connection header type
//=================
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR,12)
class IDataTunnel
{
public:
	IDataTunnel();
	~IDataTunnel();
	bool initDataTunnel();
	void startTunnelLoop();
	void stopTunnelLoop();
	void setLocalPort(int port);
	void setEndpointAddr(string addr,int port);
	bool sendVideoData(char*data,int size,bool last);
	bool sendAudioData(char*data,int size);
	bool isClientConnected() const;
	bool getControllerData(char **data,int *size);
	
private:
	SOCKET agentFd;
	SOCKADDR_IN agentLocalAddr;
	SOCKADDR_IN endpointAddr;
	bool clientConnected;
	bool runFlag;
	HANDLE g_hMutex_send_network;
	HANDLE g_hMutex_controller_network;
	queue< pair<char*,int> > controllerInformationQueue;
	bool sendConnectionResponseData();
};