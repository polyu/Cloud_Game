
#include "main.h"
//===============Flag=====================
static bool videoThreadRunFlag=true;
static bool NetworkRecvThreadRunFlag=true;
static char *NetworkThreadWatchVariable=0;
//==================Controller======================
static Controller controller;
//==============================================
static SDL_sem* videoSem=0;
static H264VideoRTPSource *videoSource;
static unsigned char videotempBuf[MAXTEMPBUF];
static unsigned char videoframeBuf[MAXFRAMEBUF];
static unsigned char videoframeCopyBuf[MAXFRAMEBUF];
static int videoframeCursor=0;
static int videocopyframeCursor=0;
static bool videoCanDecode=false;
static Groupsock *localVideoSock;
static VideoStreamDecoder vdecoder;
//===================NETWORKAUDIO======================
static queue< pair<int,char*> > audioPacketQueue;
static unsigned char audiotempBuf[MAXTEMPBUF];
static unsigned char audioplayBuf[MAXTEMPBUF];
static int audioplaycursor=0;
static SDL_sem* audioSem=0;
static BasicUDPSource *audioSource;
static Groupsock *localAudioSock;
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

static void SafeCleanUp()
{
	videoThreadRunFlag=false;
	NetworkRecvThreadRunFlag=false;
	NetworkThreadWatchVariable=(char*)'a';//Abort
	SDL_CloseAudio();
	if(videoSem!=NULL)
	{
		SDL_DestroySemaphore(videoSem);
	}
	if(audioSem!=NULL)
	{
		SDL_DestroySemaphore(audioSem);
	}
	
	exit(0);
	Sleep(1000);
}
static int SDL_VideoDisplayThread(void *)
{
	SDL_Rect rect;  
	while(videoThreadRunFlag)
	{
		Sleep(GUISLEEPTIME);
		if(SDL_SemTryWait(videoSem)==0)
		{
			if(videoCanDecode)
			{
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
			SDL_SemPost(videoSem);
		}
	}
	return 0;
}
static void afterGetAudioUnit(void *clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds)
{
	//printf("I'm getting audio %d\n",frameSize);
	
	
	if(SDL_SemTryWait(audioSem)==0)
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
		char *audioframeBuf=(char *)malloc(frameSize);
		memcpy(audioframeBuf,audiotempBuf,frameSize);
		audioPacketQueue.push(pair<int,char*>(frameSize,audioframeBuf));
		/**/
		
		SDL_SemPost(audioSem);
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
		if(SDL_SemTryWait(videoSem)==0)
		{
			
			memcpy(videoframeCopyBuf,videoframeBuf,videoframeCursor);
			videocopyframeCursor=videoframeCursor;
			videoframeCursor=0;
			videoCanDecode=true;
			SDL_SemPost(videoSem);
		}
	}
	else
	{

	}


	refreshVideo();

}
static void refreshVideo(void )
{
	if(NetworkRecvThreadRunFlag)
	videoSource->getNextFrame(videotempBuf,102400,afterGetVideoUnit,NULL,NULL,NULL);
}
static void refreshAudio(void )
{
	if(NetworkRecvThreadRunFlag)
	audioSource->getNextFrame(audiotempBuf,102400,afterGetAudioUnit,NULL,NULL,NULL);
}
static int NetworkRecvThread(void *)
{
	refreshAudio();
	refreshVideo();
	printf("Network Event Loop Start\n");
	env->taskScheduler().doEventLoop(NetworkThreadWatchVariable);
	return 0;
}
static void initControllerNetwork()
{
	if(!controller.initControllerClient())
	{
		exit(-4);
	}
}
static void decodeAudioQueue()
{
	if(SDL_SemTryWait(audioSem)==0)
	{
		////printf("Size:%d,Request:%d\n",audioPacketQueue.size(),len);
		while(audioPacketQueue.size()>0)//Decode them all
		{
			pair<int,char*> packet=audioPacketQueue.front();
			AVFrame *frame;
			int outSize;
			if(adecoder.decodeAudioFrame((char *)packet.second,packet.first,&frame,&outSize))
			{
				if(audioplaycursor+outSize>MAXTEMPBUF)
				{
					audioplaycursor=0;
				}
				memcpy(audioplayBuf+audioplaycursor,frame->data[0],outSize);
				audioplaycursor+=outSize;
			}
			free(packet.second);
			audioPacketQueue.pop();
			
		}
		SDL_SemPost(audioSem); 
	}
}
static void getAudioFromBuffer(void *udata, Uint8 *stream, int len)
{

	while(audioplaycursor<len)
	{
		if(!videoThreadRunFlag) return;
		decodeAudioQueue();
		if(audioplaycursor<len)
		{
			Sleep(1);
		}
	}
	memcpy(stream,audioplayBuf,len);
	memmove(audioplayBuf,audioplayBuf+len,audioplaycursor-len);
	audioplaycursor-=len;
	
}

static void initSDL()
{
	
	if((SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)==-1)) 
	{ 
        //printf("Could not initialize SDL: %s.\n", SDL_GetError());
       exit(-3);
    }
	atexit(SDL_Quit);
	//=============Thread======================
	videoSem=SDL_CreateSemaphore(1);
	audioSem=SDL_CreateSemaphore(1);
	if(videoSem==NULL)
	{
		exit(-6);
	}
	//=============Video=======================
	screen = SDL_SetVideoMode(RWIDTH, RHEIGHT, 32, SDL_SWSURFACE);
	if ( screen == NULL ) {
        //printf("Couldn't set 640x480x32 video mode: %s\n",SDL_GetError());
        exit(-3);
    }
	screenOverlay=SDL_CreateYUVOverlay(RWIDTH,RHEIGHT,SDL_IYUV_OVERLAY,screen);
	SDL_WM_SetCaption("Cloud Gaming",NULL);
	SDL_ShowCursor(0);
	//SDL_WM_GrabInput( SDL_GRAB_ON );
	//=============Sound
	wanted.freq = OUTPUTSAMPLERATE;//音频的频率 
	wanted.format = AUDIO_S16SYS;//数据格式为有符号16位		
	wanted.channels = 2;//双声道 
	wanted.samples = AUDIOBUFFERNUM;//采样数 
	wanted.callback = getAudioFromBuffer;//设置回调函数 
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
static void initStreamNetwork()
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
	initSDL();
	
	initStreamNetwork();
	initControllerNetwork();
	initDecoder();
	videoThreadRunFlag=true;
	NetworkRecvThreadRunFlag=true;
	NetworkThreadWatchVariable=0;
	SDL_CreateThread(NetworkRecvThread,NULL);
	SDL_CreateThread(SDL_VideoDisplayThread,NULL);
	bool quitFlag=false;
	SDL_Event event;
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