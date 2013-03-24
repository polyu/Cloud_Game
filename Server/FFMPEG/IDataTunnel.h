#include "stdafx.h"
#include <windows.h>
#include <string>
using namespace std;
#define VIDEODATAHEADERTYPE 0x1
#define AUDIODATAHEADERTYPE 0x2
#define CONTROLERDATAHEADERTYPE 0x3
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
	bool sendVideoData(char*data,int size);
	bool sendAudioData(char*data,int size);
	bool isClientConnected() const;
	bool getControllerData(char **data,int &size);
private:
	SOCKET agentFd;
	SOCKADDR_IN agentLocalAddr;
	SOCKADDR_IN endpointAddr;
	bool clientConnected;
	bool runFlag;
	HANDLE g_hMutex_network;
};