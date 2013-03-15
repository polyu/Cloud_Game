#include "stdafx.h"
#include <BasicUsageEnvironment.hh>
#include <liveMedia.hh>
#include "StreamDecoder.h"
#include <SDL.h>
#include <process.h>
#include <queue>
#include "Controller.h"
#ifdef main
#undef main
#endif
using namespace std;


static bool rtpThreadRunFlag=true;

//===================NETWORKAUDIO======================
static void refreshAudio();
static queue< pair<int,char*> > audioPacketQueue;
static unsigned char audiotempBuf[MAXTEMPBUF];
static HANDLE g_hMutex_audio = INVALID_HANDLE_VALUE; 
static BasicUDPSource *audioSource;
static Groupsock *localAudioSock;
static void afterGetAudioUnit(void *clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);
//=====================NETENV=========================
static TaskScheduler* scheduler;
static UsageEnvironment* env ;
static StreamDecoder decoder;
//========================SDL===================================
static SDL_Surface *screen;
static SDL_Overlay *screenOverlay;
static SDL_AudioSpec wanted;
static int SDL_WindowHeight=RHEIGHT;
static int SDL_WindowWidth=RWIDTH;
static void SDL_VideoDisplayThread(void *);


//==============================================================

static void afterGetAudioUnit(void *clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds)
{
	////printf("I got audio %d\n",frameSize);
	
	unsigned char *p;
	unsigned char *p_content;
	int headerLength;
	int frameCount;
	int frameSizeList[10];
	int remainSize=0;
	unsigned char adts[7]={0xFF, 0xF9, 0x50, 0x80, 0x00, 0x00, 0xFC};
	if(frameSize>100)//UDP CHECK!
	{
		p=audiotempBuf+12;//SKIP RTP HEADER
		if(*p==0)//CHECK RTP
		{
			p++;
			headerLength=*p;
			frameCount=headerLength/16;
			p_content=audiotempBuf+12+2+frameCount*2;
			remainSize=frameSize-12-2-frameCount*2;
			for(int i=0;i<frameCount;i++)
			{
				p++;
				int part1=*p;
				part1=part1<<8;
				p++;
				int part2=*p;
				frameSizeList[i]=(part1+part2)>>3;
				
			}
			for(int i=0;i<frameCount;i++)
			{
				if(remainSize>=frameSizeList[i])
				{
					
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
						int frameLen=frameSizeList[i]+7;
						frameLen <<= 5;//8bit * 2 - 11 = 5(headerSize 11bit)
						frameLen |= 0x1F;//5 bit    1            
						adts[4] = frameLen>>8;
						adts[5] = frameLen & 0xFF;
						char *audioframeBuf=(char *)malloc(frameSizeList[i]+sizeof(adts));
						memcpy(audioframeBuf,adts,sizeof(adts));
						memcpy(audioframeBuf+sizeof(adts),p_content,frameSizeList[i]);
						
						audioPacketQueue.push(pair<int,char*>(frameSizeList[i]+7,audioframeBuf));
						ReleaseMutex(g_hMutex_audio); 
					}
					p_content+=frameSizeList[i];
					remainSize-=frameSizeList[i];
				}
				else
				{
					//printf("Bad packet found\n");
					break;
				}
			}
			
		}
		
	}
	
	refreshAudio();
	
}

static void refreshAudio(void )
{
	if(rtpThreadRunFlag)
	audioSource->getNextFrame(audiotempBuf,102400,afterGetAudioUnit,NULL,NULL,NULL);
}
static void RTPNetworkThread(void *)
{
	refreshAudio();
	
	//printf("Network  EventLoop Start\n");
	env->taskScheduler().doEventLoop();
}
static void initDecoder();
static void initRTPNetwork();
static void initSDL();
static void initControllerNetwork();
static void decodeAudioFromQueue(void *udata, Uint8 *stream, int len);


static void decodeAudioFromQueue(void *udata, Uint8 *stream, int len)
{
	long performance=clock();
	int availLen=len;
	int requestLen=0;
	int cursor=0;
	if(WaitForSingleObject(g_hMutex_audio, 10)==WAIT_OBJECT_0)
	{
		////printf("Size:%d,Request:%d\n",audioPacketQueue.size(),len);
		while(audioPacketQueue.size()>0)
		{
			pair<int,char*> packet=audioPacketQueue.front();
			AVFrame *frame;
			int outSize;
			decoder.decodeAudioFrame(packet.second,packet.first,&frame,&outSize);
			requestLen+=outSize;;
			if(requestLen<=len)
			{
				memcpy(stream+cursor,frame->data[0],outSize);
				cursor+=outSize;
				audioPacketQueue.pop();
				free(packet.second);
				av_freep(&frame->data[0]);
				av_freep(&frame->data);
			}
			else
			{
				//printf("Buffer filled\n");
				av_freep(&frame->data[0]);
				av_freep(&frame->data);
				break;
			}
		}
		ReleaseMutex(g_hMutex_audio); 
	}
	printf("Performance Test:%d\n",clock()-performance);
	////printf("I am quit\n");
}

static void initSDL()
{
	
	if((SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)==-1)) 
	{ 
        //printf("Could not initialize SDL: %s.\n", SDL_GetError());
       exit(-3);
    }
	
	atexit(SDL_Quit);
	//=============Video
	screen = SDL_SetVideoMode(RWIDTH, RHEIGHT, 32, SDL_SWSURFACE);
	if ( screen == NULL ) {
        //printf("Couldn't set 640x480x32 video mode: %s\n",SDL_GetError());
        exit(-3);
    }
	screenOverlay=SDL_CreateYUVOverlay(RWIDTH,RHEIGHT,SDL_IYUV_OVERLAY,screen);
	SDL_WM_SetCaption("SOUND CLIENT",NULL);
	
	//=============Sound
	wanted.freq = 44100;//音频的频率 
	wanted.format = AUDIO_S16SYS;//数据格式为有符号16位		
	wanted.channels = 2;//双声道 
	wanted.samples = AUDIOBUFFERNUM;//采样数 
	wanted.callback = decodeAudioFromQueue;//设置回调函数 
	wanted.userdata = NULL; 
	/*if(SDL_OpenAudio(&wanted, NULL) < 0) 
	{  
		//printf( "SDL_OpenAudio: %s\n", SDL_GetError());  
		exit(-3); 
	}	  
	SDL_PauseAudio(0);*/

}
static void initDecoder()
{

	g_hMutex_audio = CreateMutex(NULL, FALSE, L"Mutex2");
	if (!g_hMutex_audio)  
    {  
        //printf("Failed to create mutex\n");
        exit(-1);
    }  
	if(!decoder.initDecorder())
	{
		//printf("Failed Init Decorder\n");
		exit(-1);
	}
	//printf("Decorder Init OK\n");
}
static void initRTPNetwork()
{
	in_addr listenAddress;
	listenAddress.s_addr=htonl(INADDR_ANY);
	scheduler = BasicTaskScheduler::createNew();
	env = BasicUsageEnvironment::createNew(*scheduler);
	
	Port rtpAudioPort(DEFAULT_RTPAUDIOPORT);
	localAudioSock=new Groupsock(*env, listenAddress,rtpAudioPort , 255);
	
	if(localAudioSock==NULL)
	{
		//printf("Init LOCAL AUDIO SOCK FAILED\n");
		exit(-2);
	}
	
	audioSource= BasicUDPSource::createNew(*env,localAudioSock);
	
	if(audioSource==NULL)
	{
		//printf("INIT Video Source Failed\n");
		exit(-2);
	}
	//printf("NETWORK INIT OK\n");
	
	
}
int main(int argc,char **argv)
{
	
	initSDL();
	initDecoder();
	initRTPNetwork();
	
	_beginthread(RTPNetworkThread,0,NULL);
	
	bool quitFlag=false;
	SDL_Event event;
	
	
	while(!quitFlag)
	{
		 Sleep(GUISLEEPTIME);
		 
		 while( SDL_PollEvent( &event ) )
		 {
                switch( event.type )
				{
             
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