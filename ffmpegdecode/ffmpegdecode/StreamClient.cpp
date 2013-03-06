#include "StreamClient.h"
static WSADATA wsaData;
static sockaddr_in localAddr;
static int localPort=DEFAULT_PORT;
static SOCKET sock_fd=-1;
static sockaddr_in remoteAddr; 
static int remoteAddrLen=sizeof(remoteAddr);
static HANDLE g_hMutex = INVALID_HANDLE_VALUE;  
static void recvWorker(void *);
static char recvBuf[RECVBUFSIZE];
static queue< pair<char*,int> > bufQueue;
int recvFrame=0;
void cleanUpStreamClient()
{
	if(sock_fd!=-1)
		closesocket(sock_fd);
	WSACleanup();
}
void setLocalPort(int port)
{
	localPort=port;
}
bool setupStreamClient()
{
	g_hMutex = CreateMutex(NULL, FALSE, L"Mutex");
	if (!g_hMutex)  
    {  
        printf("Failed to create mutex\n");
        return false;  
    }  
	int iResult;
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) 
	{
		printf("WSAStartup failed: %d\n", iResult);
		return false;
	}
	int len = sizeof(struct sockaddr_in);  
	localAddr.sin_family = AF_INET;  
	localAddr.sin_port = htons(localPort); /* ¼àÌý¶Ë¿Ú */  
	localAddr.sin_addr.s_addr = INADDR_ANY;  /* ±¾»ú */  
	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock_fd== INVALID_SOCKET)
	{
		printf("Create socket failed\n");
		return false;
	}
	iResult=bind(sock_fd, (sockaddr *)&localAddr, sizeof(localAddr)); 
	if(iResult== SOCKET_ERROR)
	{
		printf("Bind failed\n");
		return false;
	}
	int buff_size=65536;
	int tmp_len = sizeof(buff_size);
	iResult=setsockopt(sock_fd,SOL_SOCKET,SO_RCVBUF,(char*)&buff_size,tmp_len);
	if(iResult!=0) 
	{
		printf("Set Buffer Failed\n");
		return false;
	}
	printf("Network Buffer Size for UDP is %d\n",buff_size);
	_beginthread( recvWorker, 0, NULL );
	return true;
}
static void recvWorker(void *)
{
	while(true)
	{
		recvFrame++;
		int recvSize=recvfrom(sock_fd, recvBuf, RECVBUFSIZE, 0, (SOCKADDR *)&remoteAddr, &remoteAddrLen);
		
		printf("Geting %d pieces %d bytes from server\n",recvFrame,recvSize);
		if(recvSize==-1)
		{
			printf("Get an error %d\n",GetLastError());
			break;
		}
		WaitForSingleObject(g_hMutex, INFINITE);
		if(bufQueue.size()==MAXQUEUENUM)
		{
			printf("Queue is full!Empty the queue now\n");
			for(int i=0;i<MAXQUEUENUM;i++)
			{
				free(bufQueue.front().first);
				bufQueue.pop();
			}
		}
		char *data=(char*)malloc(recvSize);
		memcpy(data,recvBuf,recvSize);
		bufQueue.push(pair<char*,int>(data,recvSize));
		ReleaseMutex(g_hMutex); 
	}
}
bool recvData(pair<char*,int> &data)
{
	WaitForSingleObject(g_hMutex, INFINITE);
	if(bufQueue.size()==0)
	{
		ReleaseMutex(g_hMutex); 
		return false;
	}
	else
	{
		data=bufQueue.front();
		bufQueue.pop();
		ReleaseMutex(g_hMutex); 
		return true;
	}

}
