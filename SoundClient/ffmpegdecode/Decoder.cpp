#include "stdafx.h"
#include "Decoder.h"
static AVCodec *codec=NULL;
static AVCodecContext *c= NULL;
static AVFrame* frame=NULL;
static AVFrame* picture;
static SwsContext *img_convert_ctx=NULL;
static AVPacket avpkt;
int lastHeight=0;
int lastWidth=0;
bool decode_frame(pair<char*,int> &data,AVFrame** getframe)
{
	int len, got_frame;
	
	
	avpkt.data = (uint8_t*)data.first;
	avpkt.size=data.second;
	len = avcodec_decode_video2(c, frame, &got_frame, &avpkt);
	
	
	if(len<0)
	{
		printf("Error happen when decoding\n");
		return false;
	}
	if (got_frame) 
	{
		if(lastHeight!=frame->height || lastWidth!=frame->width)
		{
			removeSwscale();
			if(!setupSwscale())
			{
				printf("Error happen when setting up swscale!\n");
				return false;
			}
		}
		sws_scale(img_convert_ctx, frame->data, frame->linesize,0, RHEIGHT, picture->data, picture->linesize);  
		*getframe=picture;
		return true;
	}
	else
	{
		printf("Bad packet! OOP! Maybe next will be good!\n");
		return false;
	}
}
bool initDecoder()
{
	avcodec_register_all();
	codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        printf("Codec not found\n");
        return false;
    }

    c = avcodec_alloc_context3(codec);
	 if (!c) {
        printf("Could not allocate video codec context\n");
        return false;
    }

    if (avcodec_open2(c, codec, NULL) < 0) {
        printf("Could not open codec\n");
        return false;
    }
	frame = avcodec_alloc_frame();
	
    if (!frame ) {
        printf("Could not allocate video frame\n");
        return false;
    }
	av_init_packet(&avpkt);
	picture=alloc_picture(PIX_FMT_YUV420P, RWIDTH, RHEIGHT);
	return true;

   
}

static bool setupSwscale()
{
	img_convert_ctx = sws_getContext(frame->width, frame->height, c->pix_fmt, 
	RWIDTH, RHEIGHT, PIX_FMT_YUV420P, SWS_POINT, 
	NULL, NULL, NULL);
	if(img_convert_ctx == NULL) { 
	printf( "Cannot initialize the conversion context!\n"); 
	return false; 
	}
	return true;
}
static void removeSwscale()
{
	if(img_convert_ctx!=NULL)
	{
		sws_freeContext(img_convert_ctx);
		img_convert_ctx=NULL;
	}
	
}
static AVFrame *alloc_picture(enum PixelFormat pix_fmt, int width, int height)
{
	AVFrame *picture = avcodec_alloc_frame();
    if (!picture || avpicture_alloc((AVPicture *)picture, pix_fmt, width, height) < 0)
        av_freep(&picture);
    return picture;
}