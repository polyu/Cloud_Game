#include "stdafx.h"
#include "Decoder.h"
static AVCodec *codec=NULL;
static AVCodecContext *c= NULL;
static AVFrame* frame=NULL;
static AVFrame* yuvFrame=NULL;
static SwsContext *img_convert_ctx=NULL;

bool decode_frame(pair<char*,int> &data,AVFrame** getframe)
{
	int len, got_frame;
	AVPacket avpkt;
	av_init_packet(&avpkt);
	avpkt.data = (uint8_t*)data.first;
	avpkt.size=data.second;
	len = avcodec_decode_video2(c, frame, &got_frame, &avpkt);
	av_free_packet(&avpkt);
	
	if(len<0)
	{
		printf("Error happen when decoding\n");
		return false;
	}
	if (got_frame) 
	{
		*getframe=frame;
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
	yuvFrame= avcodec_alloc_frame();
    if (!frame || !yuvFrame) {
        printf("Could not allocate video frame\n");
        return false;
    }
	return true;

   
}
