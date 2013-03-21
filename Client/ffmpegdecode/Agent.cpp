#include "Agent.h"
static void agentConnectNetworkProxyFunc(evutil_socket_t fd, short what, void *arg)
{
	((Agent*)(arg))->agentConnectNetworkCallBack(what);
}

Agent::Agent()
{
	this->agentFd=INVALID_SOCKET;
	this->proxyControllerFd=INVALID_SOCKET;
	this->eventbase=0;
	this->agentEvent=0;
	this->proxyControllerEvent=0;
	this->tunnelEstablished=false;
}
Agent::~Agent()
{
	if(this->agentEvent!=0)
	{
		event_free(agentEvent);
	}
	if(this->proxyControllerEvent!=0)
	{
		event_free(proxyControllerEvent);
	}
	if(this->eventbase!=0)
	{
		event_base_free(eventbase);
	}
	if(this->agentFd=!INVALID_SOCKET)
	{
		closesocket(agentFd);
	}
	if(this->proxyControllerFd!=INVALID_SOCKET)
	{
		closesocket(proxyControllerFd);
	}
}
bool Agent::initAgent()
{
	if(this->setupSocket())
	{
		printf("Failed to setup socket\n");
		return false;
	}
	eventbase = event_base_new();
	if(eventbase==NULL)
	{
		printf("Failed to init eventbase\n");
		return false;
	}
	char handShakePacket[1300];
	for(int i=0;i<HANDSHAKEPACKETSIZE;i++)//Send Out Data
	{
		if(sendto(agentFd,handShakePacket,1300,0,(const sockaddr*)&agentEndPointAddr,sizeof(agentEndPointAddr)==SOCKET_ERROR))
		{
			printf("Send handShakePacket failed\n");
			return false;
		}
	}
	this->agentEvent=event_new(eventbase,this->agentFd,EV_READ|EV_PERSIST,agentConnectNetworkProxyFunc,this);
	timeval five_seconds = {5,0};
	event_add(agentEvent,&five_seconds);
	return true;
}

void Agent::setProxyControllerPort(int port)
{
	this->proxyControllerAddr.sin_addr.s_addr = INADDR_ANY;
	this->proxyControllerAddr.sin_family=AF_INET;
	this->proxyControllerAddr.sin_port=htons(port);
}
void Agent::setAgentPort(int agentPort)
{
	this->agentAddr.sin_addr.s_addr=INADDR_ANY;
	this->agentAddr.sin_family=AF_INET;
	this->agentAddr.sin_port=htons(agentPort);
}
void Agent::setLocalAudioPort(int port)
{
	this->localAudioAddr.sin_addr.s_addr=inet_addr(LOCALADDRESS);
	this->localAudioAddr.sin_port=htons(port);
	this->localAudioAddr.sin_family=AF_INET;
}
void Agent::setLocalVideoPort(int port)
{
	this->localVideoAddr.sin_addr.s_addr=inet_addr(LOCALADDRESS);
	this->localVideoAddr.sin_port=htons(port);
	this->localVideoAddr.sin_family=AF_INET;
}
bool Agent::setupSocket()
{
	this->agentFd=socket(AF_INET,SOCK_DGRAM,0);
	this->proxyControllerFd=socket(AF_INET,SOCK_DGRAM,0);
	if(agentFd!=INVALID_SOCKET || proxyControllerFd!=INVALID_SOCKET)
	{
		printf("Failed to get socket\n");
		return false;
	}
	if (bind(agentFd,(sockaddr*)&this->agentAddr,sizeof(agentAddr)) == SOCKET_ERROR) 
	{   
		printf("Error when bind Agent socket\n");
        return false;
    }
	if (bind(proxyControllerFd,(sockaddr*)&this->proxyControllerAddr,sizeof(proxyControllerAddr)) == SOCKET_ERROR) 
	{   
		printf("Error when bind Proxy Controller socket\n");
        return false;
    }
	int buff_size=65536;
	if(setsockopt(proxyControllerFd,SOL_SOCKET,SO_SNDBUF,(char*)&buff_size,sizeof(buff_size))==SOCKET_ERROR)
	{
		printf("SET SND Buffer failed\n");
		return false;
	}
	if(setsockopt(agentFd,SOL_SOCKET,SO_SNDBUF,(char*)&buff_size,sizeof(buff_size))==SOCKET_ERROR)
	{
		printf("SET SND Buffer failed\n");
		return false;
	}
	if(setsockopt(proxyControllerFd,SOL_SOCKET,SO_RCVBUF,(char*)&buff_size,sizeof(buff_size))==SOCKET_ERROR)
	{
		printf("SET RCV Buffer failed\n");
		return false;
	}
	if(setsockopt(agentFd,SOL_SOCKET,SO_RCVBUF,(char*)&buff_size,sizeof(buff_size))==SOCKET_ERROR)
	{
		printf("SET RCV Buffer failed\n");
		return false;
	}
	u_long nonblock=1;
	if(ioctlsocket(agentFd,FIONBIO,&nonblock)==SOCKET_ERROR)
	{
		printf("Error when set non-blocking agentFD\n");
        return false;
	}
	if(ioctlsocket(proxyControllerFd,FIONBIO,&nonblock)==SOCKET_ERROR)
	{
		printf("Error when set non-blocking controllerFd\n");
        return false;
	}
	return true;
}
void Agent::setEndPointAddress(string address,int port)
{
	this->agentEndPointAddr.sin_addr.s_addr=inet_addr(address.c_str());
	this->agentEndPointAddr.sin_family=AF_INET;
	this->agentEndPointAddr.sin_port=htons(port);
}
void Agent::agentConnectNetworkCallBack(short what)
{
		if(what & EV_READ)
		{

		}
		else if(what&EV_READ)
		{
			this->tunnelEstablished=true;//Tunnel Established
		}
		else if(what&EV_TIMEOUT)
		{
			
		}
}