#include "stdafx.h"
#include <BasicUsageEnvironment.hh>
#include <liveMedia.hh>
#include "VideoStreamDecoder.h"
#include "AudioStreamDecoder.h"
#include <SDL.h>
#include <process.h>
#include <queue>
#include "Controller.h"
using namespace std;

static bool videoThreadRunFlag=true;
static bool rtpThreadRunFlag=true;

//=====================Controller Network===================
static Controller controller;

//====================NETWORKVIDEO================
static HANDLE g_hMutex_video = INVALID_HANDLE_VALUE;  
static H264VideoRTPSource *videoSource;
static unsigned char videotempBuf[MAXTEMPBUF];
static unsigned char videoframeBuf[MAXFRAMEBUF];
static unsigned char videoframeCopyBuf[MAXFRAMEBUF];
static bool videoCanDecode=false;
static int videoframeCursor=0;
static int videocopyframeCursor=0;
static void refreshVideo();
static Groupsock *localVideoSock;
static void afterGetVideoUnit(void *clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);
static VideoStreamDecoder vdecoder;
//===================NETWORKAUDIO======================
static void refreshAudio();
static queue< pair<int,char*> > audioPacketQueue;
static unsigned char audiotempBuf[MAXTEMPBUF];
static HANDLE g_hMutex_audio = INVALID_HANDLE_VALUE; 
static BasicUDPSource *audioSource;
static Groupsock *localAudioSock;
static void afterGetAudioUnit(void *clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);
static AudioStreamDecoder adecoder;
//=====================NETENV=========================
static TaskScheduler* scheduler;
static UsageEnvironment* env ;
//========================SDL===================================
static SDL_Surface *screen;
static SDL_Overlay *screenOverlay;
static SDL_AudioSpec wanted;
static int SDL_WindowHeight=RHEIGHT;
static int SDL_WindowWidth=RWIDTH;
static void SDL_VideoDisplayThread(void *);
static void SafeCleanUp();
static void SafeCleanUp()
{
	videoThreadRunFlag=false;
	rtpThreadRunFlag=false;
	SDL_CloseAudio();
}
//==============================================================
static void SDL_VideoDisplayThread(void *)
{
	SDL_Rect rect;  
	while(videoThreadRunFlag)
	{
		 Sleep(GUISLEEPTIME);
		 if(WaitForSingleObject(g_hMutex_video, 10)==WAIT_OBJECT_0)
		 {
			 if(videoCanDecode==true)
			 {
				 ////printf("Try send this thing to decode length %d\n",videocopyframeCursor);
				 AVFrame* frame;
				 if(vdecoder.decodeVideoFrame((char*)videoframeCopyBuf,videocopyframeCursor,&frame))
				 {
					SDL_LockYUVOverlay(screenOverlay);
					screenOverlay->pixels[0]=frame->data[0];
					screenOverlay->pixels[1]=frame->data[1];
					screenOverlay->pixels[2]=frame->data[2];
					screenOverlay->pitches[0]=frame->linesize[0];
					screenOverlay->pitches[1]=frame->linesize[1];
					screenOverlay->pitches[2]=frame->linesize[2];
					rect.w = RWIDTH;  
					rect.h = RHEIGHT;  
					rect.x=0;
					rect.y=0;
					SDL_DisplayYUVOverlay(screenOverlay, &rect);
					SDL_UnlockYUVOverlay(screenOverlay);
				 }
				 videoCanDecode=false;
			 }
			 ReleaseMutex(g_hMutex_video); 
		 }
	}
}
static void afterGetAudioUnit(void *clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds)
{
	printf("I'm getting audio %d\n",frameSize);
	
	
	if(WaitForSingleObject(g_hMutex_audio, 3)==WAIT_OBJECT_0)
	{
						//printf("QUEUE:report:%d\n",audioPacketQueue.size());
		if(audioPacketQueue.size()>MAXAUDIOQUEUENUM)
		{
			int popnum=audioPacketQueue.size();
			for(int j=0;j<popnum;j++)
			{
				free(audioPacketQueue.front().second);
				audioPacketQueue.pop();
			}
		}
		AVFrame *frame;
		int outSize;
		long performance=clock();
		adecoder.decodeAudioFrame((char *)audiotempBuf,frameSize,&frame,&outSize);
		printf("Pre Decoder Performance Test:%d\n",clock()-performance);			
		char *audioframeBuf=(char *)malloc(outSize);
		memcpy(audioframeBuf,frame->data[0],outSize);
		audioPacketQueue.push(pair<int,char*>(outSize,audioframeBuf));
		ReleaseMutex(g_hMutex_audio); 
	}
					
	refreshAudio();
	
	
}
static void afterGetVideoUnit(void *clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds)
{
	////printf("Got a NAL Unit Length:%d\n",frameSize);
	if(videoframeCursor+frameSize>MAXFRAMEBUF)
	{
		//printf("BUF has been full\n");
		videoframeCursor=0;
	}
	char header[4]={0x00,0x00,0x00,0x01};
	memcpy(videoframeBuf+videoframeCursor,header,4);
	videoframeCursor+=4;
	memcpy(videoframeBuf+videoframeCursor,videotempBuf,frameSize);
	videoframeCursor+=frameSize;
	if(videoSource->curPacketMarkerBit())
	{
		if(WaitForSingleObject(g_hMutex_video, 1)==WAIT_OBJECT_0)
		{
			memcpy(videoframeCopyBuf,videoframeBuf,videoframeCursor);
			videocopyframeCursor=videoframeCursor;
			videoCanDecode=true;
			ReleaseMutex(g_hMutex_video); 
		}
		videoframeCursor=0;
	}
	else
	{

	}


	refreshVideo();

}
static void refreshVideo(void )
{
	if(rtpThreadRunFlag)
	videoSource->getNextFrame(videotempBuf,102400,afterGetVideoUnit,NULL,NULL,NULL);
}
static void refreshAudio(void )
{
	if(rtpThreadRunFlag)
	audioSource->getNextFrame(audiotempBuf,102400,afterGetAudioUnit,NULL,NULL,NULL);
}
static void RTPNetworkThread(void *)
{
	refreshAudio();
	refreshVideo();
	printf("Network Event Loop Start\n");
	env->taskScheduler().doEventLoop();
}
static void initDecoder();
static void initRTPNetwork();
static void initSDL();
static void initControllerNetwork();
static void getAudioFromQueue(void *udata, Uint8 *stream, int len);
static void initControllerNetwork()
{
	if(!controller.initControllerClient())
	{
		exit(-4);
	}
}
static void getAudioFromQueue(void *udata, Uint8 *stream, int len)
{
	int availLen=len;
	int requestLen=0;
	int cursor=0;
	if(WaitForSingleObject(g_hMutex_audio, 10)==WAIT_OBJECT_0)
	{
		////printf("Size:%d,Request:%d\n",audioPacketQueue.size(),len);
		while(audioPacketQueue.size()>0)
		{
			pair<int,char*> packet=audioPacketQueue.front();
			
			requestLen+=packet.first;;
			if(requestLen<=len)
			{
				memcpy(stream+cursor,packet.second,packet.first);
				cursor+=packet.first;
				audioPacketQueue.pop();
				free(packet.second);
			}
			else
			{
				printf("Buffer filled\n");
				break;
			}
		}
		ReleaseMutex(g_hMutex_audio); 
	}
	
}

static void initSDL()
{
	
	if((SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)==-1)) 
	{ 
        //printf("Could not initialize SDL: %s.\n", SDL_GetError());
       exit(-3);
    }
	atexit(SafeCleanUp);
	atexit(SDL_Quit);
	//=============Video
	screen = SDL_SetVideoMode(RWIDTH, RHEIGHT, 32, SDL_SWSURFACE);
	if ( screen == NULL ) {
        //printf("Couldn't set 640x480x32 video mode: %s\n",SDL_GetError());
        exit(-3);
    }
	screenOverlay=SDL_CreateYUVOverlay(RWIDTH,RHEIGHT,SDL_IYUV_OVERLAY,screen);
	SDL_WM_SetCaption("Cloud Gaming",NULL);
	SDL_ShowCursor(0);
	SDL_WM_GrabInput( SDL_GRAB_ON );
	//=============Sound
	wanted.freq = OUTPUTSAMPLERATE;//音频的频率 
	wanted.format = AUDIO_S16SYS;//数据格式为有符号16位		
	wanted.channels = 2;//双声道 
	wanted.samples = AUDIOBUFFERNUM;//采样数 
	wanted.callback = getAudioFromQueue;//设置回调函数 
	wanted.userdata = NULL; 
	if(SDL_OpenAudio(&wanted, NULL) < 0) 
	{  
		//printf( "SDL_OpenAudio: %s\n", SDL_GetError());  
		exit(-3); 
	}	  
	SDL_PauseAudio(0);

}
static void initDecoder()
{
	g_hMutex_video = CreateMutex(NULL, FALSE, L"Mutex");
	g_hMutex_audio = CreateMutex(NULL, FALSE, L"Mutex2");
	if (!g_hMutex_video || !g_hMutex_audio)  
    {  
        //printf("Failed to create mutex\n");
        exit(-1);
    }  
	if(!vdecoder.initDecorder())
	{
		printf("Failed Init Video Decorder\n");
		exit(-1);
	}
	if(!adecoder.initDecorder())
	{
		printf("Failed Init Audio Decorder\n");
		exit(-1);
	}
	
}
static void initRTPNetwork()
{
	in_addr listenAddress;
	listenAddress.s_addr=htonl(INADDR_ANY);
	scheduler = BasicTaskScheduler::createNew();
	env = BasicUsageEnvironment::createNew(*scheduler);
	Port rtpVideoPort(DEFAULT_RTPVIDEOPORT);
	localVideoSock=new Groupsock(*env, listenAddress,rtpVideoPort , 255);
	Port rtpAudioPort(DEFAULT_RTPAUDIOPORT);
	localAudioSock=new Groupsock(*env, listenAddress,rtpAudioPort , 255);
	if(localVideoSock==NULL)
	{
		//printf("Init LOCAL VIDEO SOCK FAILED\n");
		exit(-2);
	}
	if(localAudioSock==NULL)
	{
		//printf("Init LOCAL AUDIO SOCK FAILED\n");
		exit(-2);
	}
	videoSource=H264VideoRTPSource::createNew(*env,localVideoSock,96,30000);
	audioSource= BasicUDPSource::createNew(*env,localAudioSock);
	if(videoSource==NULL)
	{
		//printf("INIT Video Source Failed\n");
		exit(-2);
	}
	if(audioSource==NULL)
	{
		//printf("INIT Video Source Failed\n");
		exit(-2);
	}
	//printf("NETWORK INIT OK\n");
	
	
}
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	//initControllerNetwork();
	initSDL();
	initDecoder();
	initRTPNetwork();
	
	_beginthread(RTPNetworkThread,0,NULL);
	_beginthread(SDL_VideoDisplayThread,0,NULL);
	bool quitFlag=false;
	SDL_Event event;
	//printf("SDL Ready\n");
	
	while(!quitFlag)
	{
		 Sleep(GUISLEEPTIME);
		 
		 while( SDL_PollEvent( &event ) )
		 {
                switch( event.type )
				{
             
                    case SDL_KEYDOWN:
						if(event.key.keysym.sym==SDLK_F2)
						{
							SDL_WM_GrabInput(SDL_GRAB_OFF);
							break;
						}
						//printf("%d%d\n",event.key.keysym.sym,event.key.keysym.mod);
						controller.sendKeyEvent(event.key.keysym.sym,event.key.keysym.mod);
				    break;

					case SDL_MOUSEMOTION:
						//printf("Mouse moved by %d,%d to (%d,%d)\n",event.motion.xrel, event.motion.yrel,event.motion.x, event.motion.y);
						controller.sendMouseEvent(event.motion.xrel, event.motion.yrel,0,0);
					break;

					case SDL_MOUSEBUTTONDOWN:
						if(SDL_WM_GrabInput(SDL_GRAB_QUERY)==SDL_GRAB_OFF)
						{
							SDL_WM_GrabInput(SDL_GRAB_ON);
							break;
						}
						//printf("Mouse button %d pressed at (%d,%d,%d,%d)\n",event.button.button, event.button.x, event.button.y,event.motion.xrel,event.motion.yrel);
						controller.sendMouseEvent(0,0,event.button.button,PRESSDOWNDIRECTION);
						break;

					case SDL_MOUSEBUTTONUP:
						controller.sendMouseEvent(0,0,event.button.button,PRESSUPDIRECTION);
						break;

                    case SDL_QUIT:
                        quitFlag = true;
                        break;

                    default:
                        
						break;
				 }
		 }
	}
	SafeCleanUp();
	
	return 0;
	

}