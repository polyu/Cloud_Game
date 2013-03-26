#include "ISoundComponent.h"
#include "IVideoComponent.h"
#include "IController.h"
#include "getopt.h"
#include <process.h>
static IController controller;
static IVideoComponent vComponent;
static ISoundComponent aComponent;
static IDataTunnel tunnel;
static WSADATA wsaData;
static bool runFlag;
static void initExternLibrary();
static void shutdownServer();
static void handleArgument(int argc, char* argv[]);
static void videoComponentThread(void*);
static void audioComponentThread(void*);
static void controllerThread(void *);
static void tunnelThread(void *);
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
	WSAStartup(MAKEWORD(2,1),&wsaData);
	avcodec_register_all();
}
static void shutdownServer()
{
	tunnel.stopTunnelLoop();
	vComponent.stopFrameLoop();
	aComponent.stopFrameLoop();
	controller.stopControllerLoop();
	Sleep(2000);
	WSACleanup();
	printf("System going to shutdown\n");
	Sleep(5000);

}
static void tunnelThread(void *)
{
	tunnel.startTunnelLoop();
	runFlag=false;
}
static void videoComponentThread(void*)
{
	vComponent.startFrameLoop();
	runFlag=false;
}
static void audioComponentThread(void*)
{
	aComponent.startFrameLoop();
	runFlag=false;
}
static void controllerThread(void *)
{
	controller.startControllerLoop();
	runFlag=false;
}
static void handleArgument(int argc, char* argv[])
{
	 int c;
	 int digit_optind = 0;
	 while (true)
      {
			int this_option_optind = optind ? optind : 1;
			c = getopt (argc, argv, "q:p:");
			if (c == -1)
				break;
			
			switch (c)
			{
				case 'q':
					{
						int quality=atoi(optarg);
						switch(quality)
						{
							case 1:
								vComponent.setQuality(1024,768,8000000);
								break;
							case 2:
								vComponent.setQuality(800,600,4000000);
								break;
							case 3:
								vComponent.setQuality(640,480,2000000);
								break;
							case 4:
								vComponent.setQuality(320,240,1000000);
								break;
							default:
								vComponent.setQuality(320,240,1000000);
						}
					}
					break;
				case 'p':
					{
						int localPort=atoi(optarg);
						if(localPort>1024&&localPort<65534)
						{
							tunnel.setLocalPort(localPort);
						}
					}
					break;

				
			}
    }

}
int main(int argc, char* argv[])
{
	
	handleArgument(argc,argv);
	initExternLibrary();
	if(!tunnel.initDataTunnel())
	{
		printf("Init data tunnel failed\n");
		return -7;
	}
	if(!controller.initController())
	{
		printf("Init controller failed\n");
		return -2;
	}
	if(!vComponent.initVideoComponent())
	{
		printf("Video Component Server Failed\n");
		return -1;	
	}
	if(!aComponent.initSoundComponent())
	{
		printf("Audio Component Server Failed\n");
		return -1;	
	}
	aComponent.setDataTunnel(&tunnel);
	vComponent.setDataTunnel(&tunnel);
	controller.setDataTunnel(&tunnel);
	_beginthread(videoComponentThread,0,NULL);
	_beginthread(audioComponentThread,0,NULL);
	_beginthread(controllerThread,0,NULL);
	_beginthread(tunnelThread,0,NULL);
	runFlag=true;
	SetConsoleCtrlHandler( (PHANDLER_ROUTINE) consoleHandler, true ) ;
	while(runFlag)
	{
		Sleep(1000);
	}
	shutdownServer();
	system("pause");
	
}