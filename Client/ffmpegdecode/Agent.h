#include "stdafx.h"
#include <string>
#include <event2/event.h>
static bool setupLocalSocket();
static bool setupProxySocket();
void startAgentLoop();
/*
using namespace std;
static void agentConnectNetworkProxyFunc(evutil_socket_t fd, short what, void *arg);
class Agent
{
friend static void agentConnectNetworkProxyFunc(evutil_socket_t fd, short what, void *arg);
public:
	Agent();
	~Agent();
	bool initAgent();
	void setProxyControllerPort(int proxyControllerPort);
	void setAgentPort(int agentPort);
	void setLocalAudioPort(int port);
	void setLocalVideoPort(int port);
	void setEndPointAddress(string address,int port);
	void startAgentLoop();
private:
	SOCKET proxyControllerFd;
	SOCKET agentFd;
	SOCKADDR_IN proxyControllerAddr;
	SOCKADDR_IN agentAddr;
	SOCKADDR_IN agentEndPointAddr;
	SOCKADDR_IN localAudioAddr;
	SOCKADDR_IN localVideoAddr;
	bool setupSocket();
	event_base *eventbase;
	event *proxyControllerEvent;
	event *agentEvent;
	bool tunnelEstablished;
private:
	void agentConnectNetworkCallBack(short what);
};*/