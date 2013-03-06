#include "StreamServer.h"
StreamServer::StreamServer()
{
	this->port=DEFAULT_PORT;
	sock_fd=-1;
	sendFrame=0;
}
StreamServer::~StreamServer()
{
	if(sock_fd!=-1)
		closesocket(sock_fd);
	WSACleanup();
}
void StreamServer::setLocalPort(int port)
{
	this->port=port;
}
void StreamServer::setDestAddress(char *addr,int port)
{
  remote.sin_family = AF_INET;  
  remote.sin_port = htons(port); 
  remote.sin_addr.s_addr = inet_addr(addr);   
}
bool StreamServer::sendPacket(char* buf, int size)
{
	int sendSize=sendto(sock_fd, buf, size, 0, (struct sockaddr *)&remote, sizeof(remote));
	if( sendSize!= SOCKET_ERROR)
	{
		sendFrame++;
		printf("Send %d pieces packet in data %d\n",sendFrame,sendSize);
		return true;
	}
	else
	{
		printf("Send data failed\n");
		return false;
	}
}
bool StreamServer::startServer()
{
	
	int iResult;
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) 
	{
		printf("WSAStartup failed: %d\n", iResult);
		return false;
	}
	int len = sizeof(struct sockaddr_in);  
	local.sin_family = AF_INET;  
	local.sin_port = htons(this->port); /* ¼àÌý¶Ë¿Ú */  
	local.sin_addr.s_addr = INADDR_ANY;  /* ±¾»ú */  
	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock_fd== INVALID_SOCKET)
	{
		printf("Create socket failed\n");
		return false;
	}
	iResult=bind(sock_fd, (struct sockaddr *)&local, sizeof(local)); 
	if(iResult== SOCKET_ERROR)
	{
		printf("Bind failed\n");
		return false;
	}
	int buff_size=65536;
	int tmp_len = sizeof(buff_size);
	iResult=setsockopt(sock_fd,SOL_SOCKET,SO_SNDBUF,(char*)&buff_size,tmp_len);
	if(iResult!=0) 
	{
		printf("Set Buffer Failed\n");
		return false;
	}
	printf("Network Buffer Size for UDP is %d\n",buff_size);
	return true;
}