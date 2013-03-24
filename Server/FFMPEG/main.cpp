
#include "ISoundComponent.h"
#include "IVideoComponent.h"
#include "IController.h"
#include <process.h>


static IVideoComponent vcapturer;
static ISoundComponent acapturer;
static IController controller;
static bool runFlag;
static void initExternLibrary();
static void shutdownServer();
static void videoCaptureThread(void*);
static void audioCaptureThread(void*);
static void controllerThread(void *);
static  bool consoleHandler( DWORD fdwctrltype );
static bool consoleHandler( DWORD fdwctrltype )
{
	switch( fdwctrltype ) 
    { 

		case CTRL_C_EVENT: 

		case CTRL_CLOSE_EVENT: 

		case CTRL_BREAK_EVENT: 
 
		case CTRL_LOGOFF_EVENT: 
 
		case CTRL_SHUTDOWN_EVENT: 
			runFlag=false;
			shutdownServer();
			return true;
			
		default: 
			return false; 
    } 
	
}
static void initExternLibrary()
{
	avformat_network_init();
	av_register_all() ;
	avcodec_register_all();
}
static void shutdownServer()
{
	vcapturer.stopCapture();
	acapturer.stopFrameLoop();
	controller.stopControllerLoop();
	printf("System going to shutdown\n");
	Sleep(5000);

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
	initExternLibrary();
	if(!controller.initIController())
	{
		printf("Init controller failed\n");
		return -2;
	}
	
	printf("Video Stream Server Start\n");
	
	
	if(!vcapturer.initVideoComponent())
	{
		printf("Video Capture Server Failed\n");
		return -1;	
	}
	
	if(!acapturer.initSoundComponent())
	{
		printf("Audio Capture Server Failed\n");
		return -1;	
	}
	
	_beginthread(videoCaptureThread,0,NULL);
	_beginthread(audioCaptureThread,0,NULL);
	_beginthread(controllerThread,0,NULL);
	runFlag=true;
	SetConsoleCtrlHandler( (PHANDLER_ROUTINE) consoleHandler, true ) ;
	while(runFlag)
	{
		Sleep(1000);
	}
	
}