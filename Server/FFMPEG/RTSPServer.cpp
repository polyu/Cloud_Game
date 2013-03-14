#include "RTSPServer.h"
IRTSPServer::IRTSPServer()
{
	scheduler=0;
	env=0;
	rtspServer=0;
	rtspServerPortNum=DEFAULT_RTSPPORT;
	gamingSession=0;
}
bool IRTSPServer::initRTSPServer()
{
	 scheduler = BasicTaskScheduler::createNew();
	 env = BasicUsageEnvironment::createNew(*scheduler);
	 this->rtspServer=RTSPServer::createNew(*env, rtspServerPortNum);
	 if(this->rtspServer==NULL)
	 {
		 printf("Fail open RTSP SERVER");
		 return false;
	 }
	 this->gamingSession=ServerMediaSession::createNew(*env);
	 if(this->rtspServer==NULL)
	 {
		 printf("Fail open Session");
		 return false;
	 }
	 return true;


}