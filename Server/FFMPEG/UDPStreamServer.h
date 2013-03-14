#pragma once

#include <windows.h>
#include <stdio.h>
class UDPStreamServer
{
public:
	UDPStreamServer();
	~UDPStreamServer();
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