#include "stdafx.h"
#include <BasicUsageEnvironment.hh>
#include <liveMedia.hh>
#include "StreamDecoder.h"
#include <SDL.h>
#include <process.h>
#ifdef main
#undef main
#endif 

unsigned char tempBuf[MAXTEMPBUF];
unsigned char frameBuf[MAXFRAMEBUF];
unsigned char frameCopyBuf[MAXFRAMEBUF];
bool canDecode=false;
int frameCursor=0;
H264VideoRTPSource *videoSource;
TaskScheduler* scheduler;
UsageEnvironment* env ;
Groupsock *localVideoSock;
StreamDecoder decoder;
void afterGetUnit(void *clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);
void afterGetUnit(void *clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds)
{
	//printf("Got a NAL Unit Length:%d\n",frameSize);
	if(frameCursor+frameSize>MAXFRAMEBUF)
	{
		printf("BUF has been full\n");
		frameCursor=0;
	}
	memcpy(frameBuf+frameCursor,tempBuf,frameSize);
	frameCursor+=frameSize;
	if(videoSource->curPacketMarkerBit())
	{
		//Can decode here
		//==============
		frameCursor=0;
	}
	else
	{
		
	}
	videoSource->getNextFrame(tempBuf,102400,afterGetUnit,NULL,NULL,NULL);
}
void networkThread(void *)
{
	videoSource->getNextFrame(tempBuf,102400,afterGetUnit,NULL,NULL,NULL);
	printf("Network EventLoop Start\n");
	env->taskScheduler().doEventLoop();
}
int main(int argv,char **argc)
{
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
	localVideoSock=new Groupsock(*env, listenAddress,rtpVideoPort , 255);
	if(localVideoSock==NULL)
	{
		printf("Init LOCAL SOCK FAILED\n");
		return -1;
	}
	videoSource=H264VideoRTPSource::createNew(*env,localVideoSock,96);
	if(videoSource==NULL)
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
		 
		/*if(recvData(data))
		{
			if(decode_frame(data,&frame))
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
			free(data.first);
							
		}*/
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