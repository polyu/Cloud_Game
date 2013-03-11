#include "stdafx.h"
#include <BasicUsageEnvironment.hh>
#include <liveMedia.hh>
#include "StreamDecoder.h"
#include <SDL.h>
#include <process.h>
#include <queue>
using namespace std;
#ifdef main
#undef main
#endif 



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
void NetworkThread(void *);
static StreamDecoder decoder;
//========================SDL===================================
static SDL_Surface *screen;
static SDL_Overlay *screenOverlay;
static SDL_AudioSpec wanted;

//==============================================================
static void afterGetAudioUnit(void *clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds)
{
	printf("I got audio %d\n",frameSize);
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
					printf("Bad packet found\n");
					break;
				}
			}
			
		}
		
	}
	refreshAudio();
	
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


	refreshVideo();

}
static void refreshVideo(void )
{
	videoSource->getNextFrame(videotempBuf,102400,afterGetVideoUnit,NULL,NULL,NULL);
}
static void refreshAudio(void )
{
	audioSource->getNextFrame(audiotempBuf,102400,afterGetAudioUnit,NULL,NULL,NULL);
}
static void NetworkThread(void *)
{
	refreshAudio();
	refreshVideo();
	
	printf("Network  EventLoop Start\n");
	env->taskScheduler().doEventLoop();
}
static void initDecoder();
static void initNetwork();
static void initSDL();

static void decodeAudioFromQueue(void *udata, Uint8 *stream, int len);
static void decodeAudioFromQueue(void *udata, Uint8 *stream, int len)
{
	int availLen=len;

	if(WaitForSingleObject(g_hMutex_audio, 3)==WAIT_OBJECT_0)
	{
		while(audioPacketQueue.size()>0)
		{
			pair<int,char*> packet=audioPacketQueue.front();
			audioPacketQueue.pop();
			AVFrame *frame;
			int outSize;
			decoder.decodeAudioFrame(packet.second,packet.first,&frame,&outSize);
			SDL_MixAudio(stream,frame->data[0],outSize,SDL_MIX_MAXVOLUME);
			free(packet.second);
		}
		ReleaseMutex(g_hMutex_audio); 
	}
	//printf("I am quit\n");
}

static void initSDL()
{
	if((SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)==-1)) 
	{ 
        printf("Could not initialize SDL: %s.\n", SDL_GetError());
       exit(-3);
    }
	atexit(SDL_Quit);
	//=============Video
	screen = SDL_SetVideoMode(RWIDTH, RHEIGHT, 32, SDL_SWSURFACE);
	if ( screen == NULL ) {
        printf("Couldn't set 640x480x32 video mode: %s\n",
                        SDL_GetError());
        exit(-3);
    }
	screenOverlay=SDL_CreateYUVOverlay(RWIDTH,RHEIGHT,SDL_IYUV_OVERLAY,screen);
	SDL_WM_SetCaption("Cloud Gaming",NULL);
	//=============Sound
	wanted.freq = 44100;//音频的频率 
	wanted.format = AUDIO_S16;//数据格式为有符号16位		
	wanted.channels = 2;//双声道 
	wanted.samples = AUDIOBUFFERNUM;//采样数 
	wanted.callback = decodeAudioFromQueue;//设置回调函数 
	wanted.userdata = NULL; 
	if(SDL_OpenAudio(&wanted, NULL) < 0) 
	{  
		printf( "SDL_OpenAudio: %s\n", SDL_GetError());  
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
        printf("Failed to create mutex\n");
        exit(-1);
    }  
	if(!decoder.initDecorder())
	{
		printf("Failed Init Decorder\n");
		exit(-1);
	}
	printf("Decorder Init OK\n");
}
static void initNetwork()
{
	in_addr listenAddress;
	listenAddress.s_addr=htonl(INADDR_ANY);
	scheduler = BasicTaskScheduler::createNew();
	env = BasicUsageEnvironment::createNew(*scheduler);
	Port rtpVideoPort(DEFAULT_PORT);
	localVideoSock=new Groupsock(*env, listenAddress,rtpVideoPort , 255);
	Port rtpAudioPort(DEFAULT_PORT+VIDEOAUDIOPORTGAP);
	localAudioSock=new Groupsock(*env, listenAddress,rtpAudioPort , 255);
	if(localVideoSock==NULL)
	{
		printf("Init LOCAL VIDEO SOCK FAILED\n");
		exit(-2);
	}
	if(localAudioSock==NULL)
	{
		printf("Init LOCAL AUDIO SOCK FAILED\n");
		exit(-2);
	}
	videoSource=H264VideoRTPSource::createNew(*env,localVideoSock,96,30000);
	audioSource= BasicUDPSource::createNew(*env,localAudioSock);
	if(videoSource==NULL)
	{
		printf("INIT Video Source Failed\n");
		exit(-2);
	}
	if(audioSource==NULL)
	{
		printf("INIT Video Source Failed\n");
		exit(-2);
	}
	printf("NETWORK INIT OK\n");
	_beginthread(NetworkThread,0,NULL);
	
}
int main(int argv,char **argc)
{
	
	initDecoder();
	initNetwork();
	initSDL();

	bool quitFlag=false;
	SDL_Event event;
	SDL_Rect rect;  
	printf("SDL Ready\n");
	while(!quitFlag)
	{
		 Sleep(GUISLEEPTIME);
		 if(WaitForSingleObject(g_hMutex_video, 10)==WAIT_OBJECT_0)
		 {
			 if(videoCanDecode==true)
			 {
				 //printf("Try send this thing to decode length %d\n",videocopyframeCursor);
				 AVFrame* frame;
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
	
	SDL_CloseAudio();
	return 0;
	

}