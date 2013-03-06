#pragma once

#include <windows.h>
#include <stdio.h>
#define DEFAULT_PORT 27015
class StreamServer
{
public:
	StreamServer();
	~StreamServer();
	bool startServer();
	void setLocalPort(int port);
	void setDestAddress(char *addr,int port);
	bool sendPacket(char*, int size);
private:
	WSADATA wsaData;
	sockaddr_in local;
	int port;
	SOCKET sock_fd;
	sockaddr_in remote; 
	int sendFrame;

};