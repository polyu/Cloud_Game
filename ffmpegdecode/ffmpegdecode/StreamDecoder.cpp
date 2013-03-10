#include "StreamDecoder.h"
StreamDecoder::StreamDecoder()
{
	
	this->localPort=DEFAULT_PORT;

	this->audio_codec=0;
	this->video_codec=0;
	this->videoframe=0;
	this->videopicture=0;

	this->audioframe=0;
	this->lastWidth=0;
	this->lastHeight=0;
	this->img_convert_ctx=0;
	avcodec_register_all();

}
StreamDecoder::~StreamDecoder()
{
	if(this->video_codec_context!=0)
	{
		avcodec_close(video_codec_context);
		video_codec=0;
		
	}
	if(this->audio_codec_context!=0)
	{
		avcodec_close(audio_codec_context);
		this->audio_codec=0;
		
	}
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
	if(this->videoframe!=0)
	{
		avcodec_free_frame(&videoframe);
		videoframe=0;
	}
	if(this->audioframe!=0)
	{
		avcodec_free_frame(&videoframe);
		videoframe=0;
	}
	if(this->videopicture!=0)
	{
		avcodec_free_frame(&videopicture);
		videopicture=0;
	}
	av_free_packet(&videoavpkt);
	av_free_packet(&audioavpkt);
}
void StreamDecoder::setLocalPort(int port)
{
	this->localPort=port;
}
bool StreamDecoder::decodeAudioFrame(char*data,int size,AVFrame **getframe)
{
	int len, got_frame;
	audioavpkt.data = (uint8_t*)data;
	audioavpkt.size=size;
	len = avcodec_decode_audio4(this->audio_codec_context, audioframe, &got_frame, &audioavpkt);
	if(len<0)
	{
		printf("Error happen when decoding\n");
		return false;
	}
	if (got_frame) 
	{
		printf("Decode audio ok\n");
		return true;
	}
	else
	{
	}
	return true;
}
bool StreamDecoder::decodeVideoFrame(char*data,int size,AVFrame **getframe)
{
	int len, got_frame;
	videoavpkt.data = (uint8_t*)data;
	videoavpkt.size=size;
	len = avcodec_decode_video2(this->video_codec_context, videoframe, &got_frame, &videoavpkt);
	if(len<0)
	{
		printf("Error happen when decoding\n");
		return false;
	}
	if (got_frame) 
	{
		if(lastHeight!=videoframe->height || lastWidth!=videoframe->width)
		{
			removeSwscale();
			if(!setupSwscale())
			{
				printf("Error happen when setting up swscale!\n");
				return false;
			}
		}
		sws_scale(img_convert_ctx, videoframe->data, videoframe->linesize,0, RHEIGHT, videopicture->data, videopicture->linesize);  
		*getframe=videopicture;
		return true;
	}
	else
	{
		printf("Bad packet! OOP! Maybe next will be good!\n");
		return false;
	}
	return true;
}
bool StreamDecoder::openAudioCodec()
{
	this->audio_codec=avcodec_find_decoder(AV_CODEC_ID_AAC);
	if (!this->audio_codec) 
	{
		printf( "audio codec not found/n");
		return false;
    }
	this->audio_codec_context = avcodec_alloc_context3(this->audio_codec);
	if (avcodec_open2(this->audio_codec_context, this->audio_codec,NULL) < 0) 
	{
        printf( "could not open audio codec\n");
        return false;
    }
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
        printf( "could not open video codec\n");
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
	
		videoframe = avcodec_alloc_frame();
	audioframe = avcodec_alloc_frame();
    if (!videoframe || !audioframe) {
        printf("Could not allocate video frame\n");
        return false;
    }
	av_init_packet(&videoavpkt);
	av_init_packet(&audioavpkt);
	videopicture=alloc_picture(PIX_FMT_YUV420P, RWIDTH, RHEIGHT);
	return true;
}

bool StreamDecoder::setupSwscale()
{
	img_convert_ctx = sws_getContext(videoframe->width, videoframe->height, this->video_codec_context->pix_fmt, 
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
