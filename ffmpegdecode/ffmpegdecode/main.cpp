#include "stdafx.h"
#include <rtpsession.h>
#include <rtppacket.h>
#include <rtpudpv4transmitter.h>
#include <rtpipv4address.h>
#include <rtpsessionparams.h>
#include <rtperrors.h>
#include <BasicUsageEnvironment.hh>
#include <liveMedia.hh>
#include "StreamDecoder.h"
#include <SDL.h>
#include <process.h>

using namespace jrtplib;
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
static unsigned char audiotempBuf[MAXTEMPBUF];
static unsigned char audioframeBuf[MAXFRAMEBUF];
static unsigned char audioframeCopyBuf[MAXFRAMEBUF];
static HANDLE g_hMutex_audio = INVALID_HANDLE_VALUE; 
static bool audioCanDecode=false;
static int audioframeCursor=0;
static int audiocopyframeCursor=0;
static RTPSession audioSession;
static RTPUDPv4TransmissionParams audioTransparams;
static RTPSessionParams audioSessparams;
//=====================AUDIO=========================
static TaskScheduler* scheduler;
static UsageEnvironment* env ;
void VideoNetworkThread(void *);
void AudioNetworkThread(void *);
static StreamDecoder decoder;
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
void VideoNetworkThread(void *)
{
	refreshVideo();
	printf("Network  EventLoop Start\n");
	env->taskScheduler().doEventLoop();
}
void AudioNetworkThread(void *)
{
	while(true)
	{
		Sleep(1);
		if(audioSession.Poll()<0)
		{
			printf("Poll Audio Failed");
			return;
		}
		audioSession.BeginDataAccess();
		
		// check incoming packets
		if (audioSession.GotoFirstSourceWithData())
		{
			
				RTPPacket *pack;
				do
				{
					while ((pack = audioSession.GetNextPacket()) != NULL)
					{
						// You can examine the data here
						//printf("Got packet !\n");
						FILE *f=fopen("c:/2.dump","w");
						fwrite(pack->GetPayloadData(),pack->GetPacketLength(),1,f);
						fclose(f);
						AVFrame *audioframe;
						BYTE ADTS[7] = {0xFF, 0xF1, 0x50, 0x80, 0x00, 0x00, 0xFC};
						int aaclen=pack->GetPacketLength()-9;
						aaclen <<= 5;//8bit * 2 - 11 = 5(headerSize 11bit)
						aaclen |= 0x1F;//5 bit    1            
						ADTS[4] = aaclen>>8;
						ADTS[5] = aaclen & 0xFF;
						memcpy(audiotempBuf,ADTS,sizeof(ADTS));
						memcpy(audiotempBuf,(char *)pack->GetPayloadData()+4,pack->GetPayloadLength()-4);
						decoder.decodeAudioFrame((char*)audiotempBuf,pack->GetPacketLength()-9,&audioframe);
						// we don't longer need the packet, so
						// we'll delete it
						audioSession.DeletePacket(pack);
					}
				}
				while (audioSession.GotoNextSourceWithData());
			 
		}
		
		audioSession.EndDataAccess();

	}
}


int main(int argv,char **argc)
{
	WSADATA dat;
	WSAStartup(MAKEWORD(2,2),&dat);
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
	//==============Video Source======================
	in_addr listenAddress;
	listenAddress.s_addr=htonl(INADDR_ANY);
	scheduler = BasicTaskScheduler::createNew();
	env = BasicUsageEnvironment::createNew(*scheduler);
	Port rtpVideoPort(DEFAULT_PORT);
	localVideoSock=new Groupsock(*env, listenAddress,rtpVideoPort , 255);
	if(localVideoSock==NULL)
	{
		printf("Init LOCAL VIDEO SOCK FAILED\n");
		return -1;
	}
	videoSource=H264VideoRTPSource::createNew(*env,localVideoSock,96,30000);
	if(videoSource==NULL)
	{
		printf("INIT Video Source Failed\n");
		return -1;
	}
	//===================AUDIO JRTP=======================================
	audioSessparams.SetOwnTimestampUnit(1.0/44100.0);
	audioSessparams.SetUsePollThread(false);
	audioTransparams.SetPortbase(DEFAULT_PORT+VIDEOAUDIOPORTGAP);
	if(audioSession.Create(audioSessparams,&audioTransparams)<0)
	{
		printf("Fail init RTP\n");
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
	_beginthread(VideoNetworkThread,0,NULL);
	_beginthread(AudioNetworkThread,0,NULL);
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