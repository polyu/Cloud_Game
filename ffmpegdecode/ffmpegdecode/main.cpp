#include "stdafx.h"
#include <BasicUsageEnvironment.hh>
#include <liveMedia.hh>
#include "StreamDecoder.h"
#include <SDL.h>
#include <process.h>
#ifdef main
#undef main
#endif 
static HANDLE g_hMutex_video = INVALID_HANDLE_VALUE;  
static HANDLE g_hMutex_audio = INVALID_HANDLE_VALUE; 
static unsigned char videotempBuf[MAXTEMPBUF];
static unsigned char videoframeBuf[MAXFRAMEBUF];
static unsigned char videoframeCopyBuf[MAXFRAMEBUF];
static bool videoCanDecode=false;
static int videoframeCursor=0;
static int videocopyframeCursor=0;
static unsigned char audiotempBuf[MAXTEMPBUF];
static unsigned char audioframeBuf[MAXFRAMEBUF];
static unsigned char audioframeCopyBuf[MAXFRAMEBUF];
static bool audioCanDecode=false;
static int audioframeCursor=0;
static int audiocopyframeCursor=0;
static H264VideoRTPSource *videoSource;
static RTPSource *audioSource;
static TaskScheduler* scheduler;
static UsageEnvironment* env ;
static Groupsock *localVideoSock;
static Groupsock *localAudioSock;
static StreamDecoder decoder;
static void afterGetVideoUnit(void *clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);
static void afterGetAudioUnit(void *clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);
static void afterGetAudioUnit(void *clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds)
{
	printf("Get Audio Size:%d\n",frameSize);
	audioSource->getNextFrame(audiotempBuf,102400,afterGetAudioUnit,NULL,NULL,NULL);
}
static void afterGetVideoUnit(void *clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds)
{
	//printf("Got a NAL Unit Length:%d\n",frameSize);
	if(videoframeCursor+frameSize>MAXFRAMEBUF)
	{
		printf("BUF has been full\n");
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
	videoSource->getNextFrame(videotempBuf,102400,afterGetVideoUnit,NULL,NULL,NULL);
}
void networkThread(void *)
{
	videoSource->getNextFrame(videotempBuf,102400,afterGetVideoUnit,NULL,NULL,NULL);
	audioSource->getNextFrame(audiotempBuf,102400,afterGetAudioUnit,NULL,NULL,NULL);
	printf("Network EventLoop Start\n");
	env->taskScheduler().doEventLoop();
}
int main(int argv,char **argc)
{
	g_hMutex_video = CreateMutex(NULL, FALSE, L"Mutex");
	g_hMutex_audio = CreateMutex(NULL, FALSE, L"Mutex2");
	if (!g_hMutex_video || !g_hMutex_audio)  
    {  
        printf("Failed to create mutex\n");
        return false;  
    }  
	if(!decoder.initDecorder())
	{
		printf("Failed Init Decorder\n");
		return -1;
	}
	//==============Video And Audio Source======================
	scheduler = BasicTaskScheduler::createNew();
	env = BasicUsageEnvironment::createNew(*scheduler);
	in_addr listenAddress;
	listenAddress.s_addr=htonl(INADDR_ANY);
	Port rtpVideoPort(DEFAULT_PORT);
	Port rtpAudioPort(DEFAULT_PORT+VIDEOAUDIOPORTGAP);
	localVideoSock=new Groupsock(*env, listenAddress,rtpVideoPort , 255);
	localAudioSock=new Groupsock(*env, listenAddress,rtpAudioPort , 255);
	if(localVideoSock==NULL)
	{
		printf("Init LOCAL VIDEO SOCK FAILED\n");
		return -1;
	}
	if(localAudioSock==NULL)
	{
		printf("Init LOCAL AUDIO SOCK FAILED\n");
		return -1;
	}
	videoSource=H264VideoRTPSource::createNew(*env,localVideoSock,96);
	audioSource=RTPSource::create
	if(videoSource==NULL)
	{
		printf("INIT Video Source Failed\n");
		return -1;
	}
	if(audioSource==NULL)
	{
		printf("INIT Video Source Failed\n");
		return -1;
	}
	//==========================================================
	if((SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)==-1)) 
	{ 
        printf("Could not initialize SDL: %s.\n", SDL_GetError());
        return -1;
    }
	SDL_Surface *screen;
	atexit(SDL_Quit);
	screen = SDL_SetVideoMode(RWIDTH, RHEIGHT, 32, SDL_SWSURFACE);
	if ( screen == NULL ) {
        printf("Couldn't set 640x480x32 video mode: %s\n",
                        SDL_GetError());
        return -1;
    }
	SDL_Overlay *screenOverlay=SDL_CreateYUVOverlay(RWIDTH,RHEIGHT,SDL_IYUV_OVERLAY,screen);
	bool quitFlag=false;
	SDL_Event event;
	SDL_Rect rect;  
	AVFrame* frame;
	_beginthread(networkThread,0,NULL);
	while(!quitFlag)
	{
		 Sleep(GUISLEEPTIME);
		 if(WaitForSingleObject(g_hMutex_video, 10)==WAIT_OBJECT_0)
		 {
			 if(videoCanDecode==true)
			 {
				 //printf("Try send this thing to decode length %d\n",videocopyframeCursor);
				 
				 if(decoder.decodeVideoFrame((char*)videoframeCopyBuf,videocopyframeCursor,&frame))
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
		 while( SDL_PollEvent( &event ) )
		 {
                switch( event.type )
				{
                   
                    case SDL_KEYDOWN:
                    case SDL_KEYUP:
                       
                        break;
                    case SDL_QUIT:
                        quitFlag = true;
                        break;
                    default:
                      
						break;
				 }
		 }
	}
	return 0;
	
	
}