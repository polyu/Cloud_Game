#include "StreamServer.h"
#include "ISoundCapturer.h"
#include "IVideoCapturer.h"
#include "IController.h"
#include <process.h>
StreamServer server;
IVideoCapturer vcapturer;
ISoundCapturer acapturer;
IController controller;
bool runFlag;
static void shutdownServer();
static void videoCaptureThread(void*);
static void audioCaptureThread(void*);
static void controllerThread(void *);
static void shutdownServer()
{
	vcapturer.stopCapture();
	acapturer.stopFrameLoop();
	controller.stopControllerLoop();
}
static void videoCaptureThread(void*)
{
	vcapturer.startFrameLoop();
	runFlag=false;
}
static void audioCaptureThread(void*)
{
	acapturer.startFrameLoop();
	runFlag=false;
}

static void controllerThread(void *)
{
	controller.startControllerLoop();
	runFlag=false;
}
int main(int argc, char* argv[])
{
	/*if(!controller.initIController())
	{
		printf("Init controller failed\n");
		return -2;
	}*/
	if(!server.initStreamServer())
	{
		printf("Stream Server Failed\n");
		return -1;	
	}
	printf("Stream Server Start\n");
	if(!vcapturer.initVideoCapture())
	{
		printf("Video Capture Server Failed\n");
		return -1;	
	}
	vcapturer.setStreamServer(&server);
	if(!acapturer.initISoundCapturer())
	{
		printf("Audio Capture Server Failed\n");
		return -1;	
	}
	acapturer.setStreamServer(&server);
	_beginthread(videoCaptureThread,0,NULL);
	//_beginthread(audioCaptureThread,0,NULL);
	//_beginthread(controllerThread,0,NULL);
	runFlag=true;
	while(runFlag)
	{
		Sleep(1000);
	}
	shutdownServer();
	system("pause");
}