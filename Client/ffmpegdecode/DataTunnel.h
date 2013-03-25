#pragma once
#include "stdafx.h"
#include <windows.h>
#include <string>
using namespace std;

//===========Explation
#define VIDEODATAHEADERTYPE 0x1
#define AUDIODATAHEADERTYPE 0x2
#define CONTROLERDATAHEADERTYPE 0x4
//0T00 0TTT From left to right, second bit show whether the last frame, From right to left, show the type of packet!
//like Video Audio Controller
//=================
class DataTunnel
{
public:
	DataTunnel();
	~DataTunnel();
	bool initDataTunnel();
	void startTunnelLoop();
	void stopTunnelLoop();
	void setLocalPort(int port);
	void setEndpointAddr(string addr,int port);
	bool isServerConnected() const;
	bool sendControllerData(char *data,int size);
private:
	SOCKET agentFd;
	SOCKADDR_IN agentLocalAddr;
	SOCKADDR_IN endpointAddr;
	bool serverConnected;
	bool runFlag;
	HANDLE g_hMutex_audio_network;
	HANDLE g_hMutex_video_network;
	HANDLE g_hMutex_send_network;
};