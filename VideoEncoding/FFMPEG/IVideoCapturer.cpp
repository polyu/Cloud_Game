#include "IVideoCapturer.h"
IVideoCapturer::IVideoCapturer()
{
	
	frameCounter=0;
	fpsClock=clock();
	lastWidth=0;
	lastHeight=0;
	lpvMem=NULL;
	hMapObject=NULL;
	rawFrame=NULL;
	img_convert_ctx=NULL;
	streamServer=NULL;
	workingThread=false;
}
IVideoCapturer::~IVideoCapturer()
{
	if(rawFrame!=NULL)
	{
		av_freep(&rawFrame->data[0]);
		avcodec_free_frame(&rawFrame);
		rawFrame=NULL;
	}
	removeSwscale();
	uninstallSharedMemory();
}
void IVideoCapturer::setStreamServer(StreamServer * streamServer)
{
	this->streamServer=streamServer;
}
bool IVideoCapturer::isMemoryReadable()
{
	return lpvMem[SHAREDMEMSIZE/8-1]==1;
}
void IVideoCapturer::setMemoryWritable()
{
	lpvMem[SHAREDMEMSIZE/8-1]=0;
}
void IVideoCapturer::uninstallSharedMemory()
{
	if(lpvMem!=NULL)
	UnmapViewOfFile(lpvMem);
	lpvMem=NULL;
	if(hMapObject!=NULL)
	CloseHandle(hMapObject);
	hMapObject=NULL;
	
	
}
bool IVideoCapturer::setupSharedMemory()
{
		hMapObject = CreateFileMapping( 
                INVALID_HANDLE_VALUE,   // use paging file
                NULL,                   // default security attributes
                PAGE_READWRITE,         // read/write access
                0,                      // size: high 32-bits
                SHAREDMEMSIZE,              // size: low 32-bits
                TEXT("ded9dllmemfilemap")); // name of map object
        if (this->hMapObject == NULL) 
            return FALSE; 
		bool fInit = (GetLastError() != ERROR_ALREADY_EXISTS); 
		lpvMem = (BYTE*)MapViewOfFile( 
                hMapObject,     // object to map view of
                FILE_MAP_WRITE, // read/write access
                0,              // high offset:  map from
                0,              // low offset:   beginning
                0);             // default: map entire file
		if (this->lpvMem == NULL) 
           return FALSE; 
		if (fInit) 
           memset(lpvMem, '\0', SHAREDMEMSIZE);
		
		return TRUE;
}

bool IVideoCapturer::initVideoCapture()
{
	if(!setupSharedMemory())
	{
		printf("Memory Failed\n");
		return false;
	}
	
	printf("Video Capture Init Great!\n");
	workingThread=true;
	return true;
}
void IVideoCapturer::startFrameLoop()
{
	while(workingThread)
	{
		//if(isMemoryReadable())
		if(streamServer!=NULL&&isMemoryReadable())
		{	
			
			int copySize=0;
			int height=0;
			int width=0;
			int bpp=0;
			memcpy((void *)&copySize,lpvMem+(SHAREDMEMSIZE-RESERVEDMEMORY)/8,sizeof(int));
			memcpy((void *)&height,lpvMem+(SHAREDMEMSIZE-RESERVEDMEMORY)/8+sizeof(height),sizeof(height));
			memcpy((void *)&width,lpvMem+(SHAREDMEMSIZE-RESERVEDMEMORY)/8+sizeof(height)*2,sizeof(width));
			memcpy((void *)&bpp,lpvMem+(SHAREDMEMSIZE-RESERVEDMEMORY)/8+sizeof(height)*3,sizeof(bpp));
			int fps=1000/((clock()-fpsClock)+1);//avoid divide 0
			fpsClock=clock();
			printf("%d bytes height:%d width:%d bpp:%d FPS:%d \n",copySize,height,width,bpp,fps);
			if(bpp!=4)
			{
				printf("Encoder cannot handle this format\n");
				workingThread=false;
				return;
			}
			
			//===============RGB32toYUV420P===================
			if(lastWidth!=width||lastHeight!=height)
			{
				printf("Swscale Rebuilt\n");
				lastWidth=width;
				lastHeight=height;
				this->removeSwscale();
				if(!this->setupSwscale(width,height))
				{
					printf("Swscale failed\n");
					workingThread=false;
					return;
				}	

			}
			if(rawFrame==NULL)
			{
				rawFrame=allocFrame(PIX_FMT_YUV420P, RWIDTH, RHEIGHT);
			}
			uint8_t *rgb_src[3]={lpvMem,NULL,NULL};
			int rgb_stride[3]={4*width, 0, 0};
			sws_scale(img_convert_ctx, rgb_src, rgb_stride, 0, height, rawFrame->data, rawFrame->linesize);
			//=============Write encoded frame and send=============================
			this->streamServer->write_video_frame(rawFrame);
			//==========================================
			//
			setMemoryWritable();
		}
		else
		{
			//printf("Try to see if i sleep too much");
			Sleep(1000/MAXFPS);
		}
	}
}
void IVideoCapturer::stopCapture()
{
	workingThread=false;
}
bool IVideoCapturer::setupSwscale(int in_width,int in_height)
{
	img_convert_ctx = sws_getContext(in_width, in_height, PIX_FMT_RGB32, 
	RWIDTH, RHEIGHT, PIX_FMT_YUV420P, SWS_POINT, 
	NULL, NULL, NULL);
	if(img_convert_ctx == NULL) { 
	printf( "Cannot initialize the conversion context!\n"); 
	return false; 
	}
	return true;
}
void IVideoCapturer::removeSwscale()
{
	if(img_convert_ctx!=NULL)
	{
		sws_freeContext(img_convert_ctx);
		img_convert_ctx=NULL;
	}
	
}
AVFrame *IVideoCapturer::allocFrame(enum PixelFormat pix_fmt, int width, int height)
{
	AVFrame *picture = avcodec_alloc_frame();
    if (!picture || avpicture_alloc((AVPicture *)picture, pix_fmt, width, height) < 0)
        av_freep(&picture);
	picture->pts=0;
    return picture;
}



