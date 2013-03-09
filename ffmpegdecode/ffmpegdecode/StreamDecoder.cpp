#include "StreamDecoder.h"
StreamDecoder::StreamDecoder()
{
	
	this->localPort=DEFAULT_PORT;

	this->audio_codec=0;
	this->video_codec=0;
	this->frame=0;

	this->lastWidth=0;
	this->lastHeight=0;
	this->img_convert_ctx=0;
	avcodec_register_all();

}
StreamDecoder::~StreamDecoder()
{
	if(this->audio_codec!=0)
	{
		av_freep(this->audio_codec);
		this->audio_codec=0;
	}
	if(this->video_codec!=0)
	{
		av_freep(this->video_codec);
		this->video_codec=0;
	}
	this->removeSwscale();
}
void StreamDecoder::setLocalPort(int port)
{
	this->localPort=port;
}

bool StreamDecoder::decodeVideoFrame(AVFrame **getframe)
{
	return true;
}
bool StreamDecoder::openAudioCodec()
{
	return true;
}
bool StreamDecoder::openVideoCodec()
{

	this->video_codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!this->video_codec) 
	{
		printf( "video codec not found/n");
		return false;
    }
	this->video_codec_context = avcodec_alloc_context3(this->video_codec);
	if (avcodec_open2(this->video_codec_context, this->video_codec,NULL) < 0) 
	{
        printf( "could not open codec\n");
        return false;
    }
	return true;
}

bool StreamDecoder::initDecorder()
{
	
	if(!this->openVideoCodec())
	{
		printf("Can open add video stream@!\n");
	    return false;
	}
	if(!this->openAudioCodec())
	{
		printf("Can open add video stream@!\n");
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

bool StreamDecoder::setupSwscale()
{
	img_convert_ctx = sws_getContext(frame->width, frame->height, this->video_codec_context->pix_fmt, 
	RWIDTH, RHEIGHT, PIX_FMT_YUV420P, SWS_POINT, 
	NULL, NULL, NULL);
	if(img_convert_ctx == NULL) { 
	printf( "Cannot initialize the conversion context!\n"); 
	return false; 
	}
	return true;
}
void StreamDecoder::removeSwscale()
{
	if(img_convert_ctx!=NULL)
	{
		sws_freeContext(img_convert_ctx);
		img_convert_ctx=NULL;
	}
	
}
AVFrame *StreamDecoder::alloc_picture(enum PixelFormat pix_fmt, int width, int height)
{
	AVFrame *picture = avcodec_alloc_frame();
    if (!picture || avpicture_alloc((AVPicture *)picture, pix_fmt, width, height) < 0)
        av_freep(&picture);
    return picture;
}
