#include "stdafx.h"
#include <BasicUsageEnvironment.hh>
#include <liveMedia.hh>
#include "StreamDecoder.h"
#include <SDL.h>
#include <process.h>

#ifdef main
#undef main
#endif 
//====================VIDEO================

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
//===================AUDIO======================
static void refreshAudio();
static unsigned char audiotempBuf[MAXTEMPBUF];
static unsigned char audioframeBuf[MAXFRAMEBUF];
static unsigned char audioframeCopyBuf[MAXFRAMEBUF];
static HANDLE g_hMutex_audio = INVALID_HANDLE_VALUE; 
static bool audioCanDecode=false;
static int audioframeCursor=0;
static int audiocopyframeCursor=0;
static BasicUDPSource *audioSource;
static Groupsock *localAudioSock;
static void afterGetAudioUnit(void *clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);
//=====================AUDIO=========================
static TaskScheduler* scheduler;
static UsageEnvironment* env ;
void NetworkThread(void *);
static StreamDecoder decoder;

#include <iostream>
using namespace std;
static void afterGetAudioUnit(void *clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds)
{
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
				cout<<"This is packet size "<<frameSizeList[i]<<endl;
			}
			for(int i=0;i<frameCount;i++)
			{
				if(remainSize>=frameSizeList[i])
				{
					int frameLen=frameSizeList[i]+7;
					frameLen <<= 5;//8bit * 2 - 11 = 5(headerSize 11bit)
					frameLen |= 0x1F;//5 bit    1            
					adts[4] = frameLen>>8;
					adts[5] = frameLen & 0xFF;
					memcpy(audioframeBuf,adts,sizeof(adts));
					memcpy(audioframeBuf+sizeof(adts),p_content,frameSizeList[i]);
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
		refreshAudio();
	}
	/*AVFrame *frame;
	unsigned char ADTS[] = {0xFF, 0xF9, 0x50, 0x80, 0x00, 0x00, 0xFC}; 
	int aacLen=frameSize - 4 + 7;
	aacLen <<= 5;//8bit * 2 - 11 = 5(headerSize 11bit)
    aacLen |= 0x1F;//5 bit    1            
    ADTS[4] = aacLen>>8;
    ADTS[5] = aacLen & 0xFF;
	memcpy(audioframeBuf, ADTS, sizeof(ADTS));
	memcpy(audioframeBuf+7,audiotempBuf+4,frameSize-4);
	decoder.decodeAudioFrame((char *)audioframeBuf,frameSize,&frame);
	refreshAudio();*/
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
void refreshVideo(void )
{
	videoSource->getNextFrame(videotempBuf,102400,afterGetVideoUnit,NULL,NULL,NULL);
}
void refreshAudio(void )
{
	audioSource->getNextFrame(audiotempBuf,102400,afterGetAudioUnit,NULL,NULL,NULL);
}
void NetworkThread(void *)
{
	refreshAudio();
	refreshVideo();
	
	printf("Network  EventLoop Start\n");
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
	in_addr listenAddress;
	listenAddress.s_addr=htonl(INADDR_ANY);
	//==============Video And Audio Source======================
	scheduler = BasicTaskScheduler::createNew();
	env = BasicUsageEnvironment::createNew(*scheduler);
	Port rtpVideoPort(DEFAULT_PORT);
	localVideoSock=new Groupsock(*env, listenAddress,rtpVideoPort , 255);
	Port rtpAudioPort(DEFAULT_PORT+VIDEOAUDIOPORTGAP);
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

	videoSource=H264VideoRTPSource::createNew(*env,localVideoSock,96,30000);
	audioSource= BasicUDPSource::createNew(*env,localAudioSock);

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
	//========================================================
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
	_beginthread(NetworkThread,0,NULL);
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