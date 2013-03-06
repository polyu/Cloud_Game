#include "stdafx.h"
#include "Player.h"
static AVCodec *codec=NULL;
static AVCodecContext *c= NULL;
static AVFrame* frame=NULL;
bool decode_frame(pair<char*,int> &data)
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
		FILE *f;
		int i;
		f=fopen("c:/1.pgm","w");
		fprintf(f,"P5\n%d %d\n%d\n",c->width, c->height,255);
		for(i=0;i<c->height;i++)
			fwrite(frame->data[0] + i * frame->linesize[0],1,c->width,f);
		fclose(f);
		 return true;
	}
	else
	{
		printf("Bad packet\n");
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
    if (!frame) {
        printf("Could not allocate video frame\n");
        return false;
    }
	return true;

   
}
static void pgm_save()
{

}