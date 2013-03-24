#include "IDataTunnel.h"
IDataTunnel::IDataTunnel()
{
	this->agentFd=INVALID_SOCKET;
	this->clientConnected=false;
	this->runFlag=false;
	this->g_hMutex_network= INVALID_HANDLE_VALUE;  
	setLocalPort(DEFAULT_LOCALPORT);
}
IDataTunnel::~IDataTunnel()
{
	if(this->agentFd!=INVALID_SOCKET)
	{
		closesocket(this->agentFd);
	}
}
void IDataTunnel::setLocalPort(int port)
{
	this->agentLocalAddr.sin_addr.s_addr=INADDR_ANY;
	this->agentLocalAddr.sin_family=AF_INET;
	this->agentLocalAddr.sin_port=htons(port);
}
void IDataTunnel::setEndpointAddr(string address,int port)
{
	this->endpointAddr.sin_addr.s_addr=inet_addr(address.c_str());
	this->endpointAddr.sin_family=AF_INET;
	this->endpointAddr.sin_port=htons(port);
}
bool IDataTunnel::isClientConnected() const
{
	return this->clientConnected;
}
bool IDataTunnel::sendAudioData(char *data,int size)
{
	char *tmpBuf=(char*)malloc(size+4);
	tmpBuf[0]=AUDIODATAHEADERTYPE;
	memcpy(tmpBuf+4,data,size);
	int ret=SOCKET_ERROR;
	if(WaitForSingleObject(this->g_hMutex_network,INFINITE)==WAIT_OBJECT_0)
	{
		ret=sendto(this->agentFd,tmpBuf,size+4,0,(const sockaddr*)&this->endpointAddr,sizeof(this->endpointAddr));
		ReleaseMutex(g_hMutex_network); 
	}
	free(tmpBuf);
	if(ret==SOCKET_ERROR)
	{
		printf("Error Happen in sending audio data:%d\n",WSAGetLastError());
		return false;
	}
	return true;
}
bool IDataTunnel::initDataTunnel()
{
	g_hMutex_network = CreateMutex(NULL, FALSE, L"Mutex");
	if(this->g_hMutex_network==INVALID_HANDLE_VALUE)
	{
		printf("Failed to init network mutex\n");
		return false;
	}
	this->agentFd=socket(AF_INET,SOCK_DGRAM,0);
	if(agentFd==INVALID_SOCKET )
	{
		printf("Failed to get socket\n");
		return false;
	}
	if (bind(agentFd,(sockaddr*)&this->agentLocalAddr,sizeof(agentLocalAddr)) == SOCKET_ERROR) 
	{   
		printf("Error when bind Agent socket\n");
        return false;
    }
	int buff_size=65536;
	if(setsockopt(agentFd,SOL_SOCKET,SO_RCVBUF,(char*)&buff_size,sizeof(buff_size))==SOCKET_ERROR)
	{
		printf("SET SND Buffer failed\n");
		return false;
	}
	if(setsockopt(agentFd,SOL_SOCKET,SO_SNDBUF,(char*)&buff_size,sizeof(buff_size))==SOCKET_ERROR)
	{
		printf("SET SND Buffer failed\n");
		return false;
	}
	u_long nonblock=1;
	if(ioctlsocket(agentFd,FIONBIO,&nonblock)==SOCKET_ERROR)
	{
		printf("Error when set non-blocking agentFD\n");
        return false;
	}
	this->runFlag=true;
	return true;
}
void IDataTunnel::startTunnelLoop()
{
	char buf[10240];
	while(runFlag)
	{
		int fromlen=sizeof(this->endpointAddr);
		int size=recvfrom(this->agentFd,buf,10240,0,(sockaddr *)&this->endpointAddr,&fromlen);
		if(size==SOCKET_ERROR)
		{
			printf("Network Error when recv from udp port\n");
			runFlag=false;
			return;
		}
		this->clientConnected=true;
	}
}
void IDataTunnel::stopTunnelLoop()
{
	runFlag=false;
}