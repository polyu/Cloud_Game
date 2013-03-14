#include <RTSPServer.hh>
#include <BasicUsageEnvironment.hh>
#include "stdafx.h"
class IRTSPServer
{
public:
	IRTSPServer();
	~IRTSPServer();
	bool initRTSPServer();
private:
	portNumBits rtspServerPortNum;
	TaskScheduler* scheduler ;
	UsageEnvironment* env ;
	RTSPServer* rtspServer;
	ServerMediaSession *gamingSession;
};