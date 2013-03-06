#include "VideoEncoder.h"


VideoEncoder::VideoEncoder()
{
	/*
	f = fopen("c:/record.avi", "wb");
    if (!f) {
        fprintf(stderr, "could not open /n");
       
    }*/
	frameCounter=0;
	encodingPerformanceTime=0;
	
	lastWidth=0;
	lastHeight=0;
	lpvMem=NULL;
	hMapObject=NULL;
	//=========Remebem to free above var
	codec=NULL;
    c= NULL;
	picture=NULL;
	
	picture_buf=NULL;
	img_convert_ctx=NULL;
	//=================================
	
	streamServer=NULL;
	workingThread=false;
}
VideoEncoder::~VideoEncoder()
{
	if(c!=NULL)
	{
		avcodec_close(c);
		av_free(c);
		c=NULL;
	}
	if(picture!=NULL)
	{
		av_freep(&picture->data[0]);
		avcodec_free_frame(&picture);
		picture=NULL;
	}
	if(picture_buf!=NULL)
	{
		free(picture_buf);
		picture_buf=NULL;
	}
	removeSwscale();
	uninstallSharedMemory();
}
void VideoEncoder::setStreamServer(StreamServer * streamServer)
{
	this->streamServer=streamServer;
}
bool VideoEncoder::isMemoryReadable()
{
	return lpvMem[SHAREDMEMSIZE/8-1]==1;
}
void VideoEncoder::setMemoryWritable()
{
	lpvMem[SHAREDMEMSIZE/8-1]=0;
}
void VideoEncoder::uninstallSharedMemory()
{
	if(lpvMem!=NULL)
	UnmapViewOfFile(lpvMem);
	lpvMem=NULL;
	if(hMapObject!=NULL)
	CloseHandle(hMapObject);
	hMapObject=NULL;
	
	
}
bool VideoEncoder::setupSharedMemory()
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
bool VideoEncoder::initVideoCodec()
{
	avcodec_register_all();
	codec = avcodec_find_encoder(CODEC_ID_H264);
    if (!codec) 
	{
        printf( "codec not found/n");
		return false;
    }
	
	
	
    c = avcodec_alloc_context3(codec);
	if (!c) 
	{
        printf("Could not allocate video codec context\n");
        return false;
    }
	/* put sample parameters */
    c->bit_rate = 3000000;
    /* resolution must be a multiple of two */
    c->width = RWIDTH;
    c->height = RHEIGHT;
    /* frames per second */
    
	c->gop_size=0;
	c->max_b_frames=0;
    c->pix_fmt = PIX_FMT_YUV420P;
	c->me_range = 16;
    c->max_qdiff = 4;
    c->qmin = 10;
    c->qmax = 51;
    c->qcompress = (0.7f);
	c->refs=1;
	//c->dia_size=1;
	//c->keyint_min=46;
	c->active_thread_type= FF_THREAD_SLICE;
	c->thread_type=FF_THREAD_SLICE;
	c->thread_count=4;
	c->slices=4;
	
	c->rc_max_rate=5000000;
	c->rc_buffer_size=200000;
	av_opt_set(c->priv_data, "tune", "zerolatency", 0);
	//av_dict_set(c->priv_data, "vprofile", "main", 0);
	av_opt_set(c->priv_data, "preset","veryfast",0);
	av_opt_set(c->priv_data,"intra-refresh","1",0);
	//c->slice_count=4;
    if (avcodec_open2(c, codec,NULL) < 0) 
	{
        printf( "could not open codec\n");
        return false;
    }
	
	return true;
}
void VideoEncoder::debugEncoder(const char *filename)
{
    AVCodec *codec;
    AVCodecContext *c= NULL;
    int i, ret, x, y, got_output;
    FILE *f;
    AVFrame *frame;
    AVPacket pkt;
    uint8_t endcode[] = { 0, 0, 1, 0xb7 };

    printf("Encode video file %s\n", filename);

    /* find the mpeg1 video encoder */
    codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    /* put sample parameters */
    c->bit_rate = 400000;
    /* resolution must be a multiple of two */
    c->width = 352;
    c->height = 288;
    /* frames per second */
    
    c->gop_size = 10; /* emit one intra frame every ten frames */
    c->max_b_frames=1;
    c->pix_fmt = AV_PIX_FMT_YUV420P;

    
        av_opt_set(c->priv_data, "preset", "slow", 0);

    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }

    frame = avcodec_alloc_frame();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
    frame->format = c->pix_fmt;
    frame->width  = c->width;
    frame->height = c->height;

    /* the image can be allocated by any means and av_image_alloc() is
     * just the most convenient way if av_malloc() is to be used */
    ret = av_image_alloc(frame->data, frame->linesize, c->width, c->height,
                         c->pix_fmt, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate raw picture buffer\n");
        exit(1);
    }

    /* encode 1 second of video */
    for(i=0;i<25;i++) {
        av_init_packet(&pkt);
        pkt.data = NULL;    // packet data will be allocated by the encoder
        pkt.size = 0;

        fflush(stdout);
        /* prepare a dummy image */
        /* Y */
        for(y=0;y<c->height;y++) {
            for(x=0;x<c->width;x++) {
                frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
            }
        }

        /* Cb and Cr */
        for(y=0;y<c->height/2;y++) {
            for(x=0;x<c->width/2;x++) {
                frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
                frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
            }
        }

        frame->pts = i;

        /* encode the image */
        ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
        if (ret < 0) {
            fprintf(stderr, "Error encoding frame\n");
            exit(1);
        }

        if (got_output) {
            printf("Write frame %3d (size=%5d)\n", i, pkt.size);
            fwrite(pkt.data, 1, pkt.size, f);
            av_free_packet(&pkt);
        }
    }

    /* get the delayed frames */
    for (got_output = 1; got_output; i++) {
        fflush(stdout);

        ret = avcodec_encode_video2(c, &pkt, NULL, &got_output);
        if (ret < 0) {
            fprintf(stderr, "Error encoding frame\n");
            exit(1);
        }

        if (got_output) {
            printf("Write frame %3d (size=%5d)\n", i, pkt.size);
            fwrite(pkt.data, 1, pkt.size, f);
            av_free_packet(&pkt);
        }
    }

    /* add sequence end code to have a real mpeg file */
    fwrite(endcode, 1, sizeof(endcode), f);
    fclose(f);

    avcodec_close(c);
    av_free(c);
    av_freep(&frame->data[0]);
    avcodec_free_frame(&frame);
    printf("\n");
}
bool VideoEncoder::initEncoder()
{
	if(!setupSharedMemory())
	{
		printf("Memory Failed\n");
		return false;
	}
	if(!initVideoCodec())
	{
		printf("Codec Failed\n");
		return false;
	}
	printf("Video Encoder Init Great!\n");
	workingThread=true;
	return true;
}
void VideoEncoder::encodeFrameLoop()
{
	
	while(workingThread)
	{
		if(isMemoryReadable()&&streamServer!=NULL)
		{	
			
			int copySize=0;
			int height=0;
			int width=0;
			int bpp=0;
			memcpy((void *)&copySize,lpvMem+(SHAREDMEMSIZE-RESERVEDMEMORY)/8,sizeof(int));
			memcpy((void *)&height,lpvMem+(SHAREDMEMSIZE-RESERVEDMEMORY)/8+sizeof(height),sizeof(height));
			memcpy((void *)&width,lpvMem+(SHAREDMEMSIZE-RESERVEDMEMORY)/8+sizeof(height)*2,sizeof(width));
			memcpy((void *)&bpp,lpvMem+(SHAREDMEMSIZE-RESERVEDMEMORY)/8+sizeof(height)*3,sizeof(bpp));
			printf("%d bytes height:%d width:%d bpp:%d \n",copySize,height,width,bpp);
			if(bpp!=4)
			{
				printf("Encoder cannot handle this format\n");
				return;
			}
			encodingPerformanceTime=clock();
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
					return;
				}	

			}
			if(picture==NULL)
			{
				picture=alloc_picture(PIX_FMT_YUV420P, RWIDTH, RHEIGHT);
			}
			uint8_t *rgb_src[3]={lpvMem,NULL,NULL};
			int rgb_stride[3]={4*width, 0, 0};
			sws_scale(img_convert_ctx, rgb_src, rgb_stride, 0, height, picture->data, picture->linesize);
			//=============Write encoded frame and send=============================
			/*pgm_save(picture->data[0], picture->linesize[0],
				c->width, c->height, "c:/2.pgm");*/
			int ret,getOutput;
			av_init_packet(&pkt);
			pkt.data = NULL;    // packet data will be allocated by the encoder
			pkt.size = 0;
			
			ret = avcodec_encode_video2(c, &pkt, picture,&getOutput);
		
			if (ret < 0) 
			{
				printf("Error encoding frame!Found A way to deal!\n");
				return;
			}
			if(getOutput)
			{
				printf("Encoding perfomance:%f\n",(float)1000/(clock()-encodingPerformanceTime));
				streamServer->sendPacket((char*)(pkt.data),pkt.size);
				av_free_packet(&pkt);
			}
			
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
void VideoEncoder::stopEncode()
{
	workingThread=false;
}
bool VideoEncoder::setupSwscale(int in_width,int in_height)
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
void VideoEncoder::removeSwscale()
{
	if(img_convert_ctx!=NULL)
	{
		sws_freeContext(img_convert_ctx);
		img_convert_ctx=NULL;
	}
	
}
AVFrame *VideoEncoder::alloc_picture(enum PixelFormat pix_fmt, int width, int height)
{
	AVFrame *picture = avcodec_alloc_frame();
    if (!picture || avpicture_alloc((AVPicture *)picture, pix_fmt, width, height) < 0)
        av_freep(&picture);
    return picture;
}

void VideoEncoder::pgm_save(unsigned char *buf, int wrap, int xsize, int ysize,
                     char *filename)
{
    FILE *f;
    int i;
    f=fopen(filename,"w");
    fprintf(f,"P5\n%d %d\n%d\n",xsize,ysize,255);
    for(i=0;i<ysize;i++)
        fwrite(buf + i * wrap,1,xsize,f);
    fclose(f);
}

