#include "IAudioStream.h"
IAudioStream::IAudioStream()
{
	
	this->audio_codec=0;
	this->audio_codec_context=0;
	
	
	
}
IAudioStream::~IAudioStream()
{
	
	cleanup();
}

bool IAudioStream::openAudioStream()
{
	this->audio_codec = avcodec_find_encoder(CODEC_ID_OPUS);
	if (!this->audio_codec) 
	{
		printf( "aideo codec not found/n");
		return false;
    }
	
	this->audio_codec_context=avcodec_alloc_context3(this->audio_codec);
	this->audio_codec_context->sample_fmt  = AV_SAMPLE_FMT_S16;
    this->audio_codec_context->bit_rate    = 64000;
    this->audio_codec_context->sample_rate = OUTPUTSAMPLERATE;
    this->audio_codec_context->channels    = 2;
	this->audio_codec_context->channel_layout=AV_CH_LAYOUT_STEREO;
	if( avcodec_open2(this->audio_codec_context, this->audio_codec, NULL)<0)
	{
		printf( "could not open audio codec\n");
        return false;
	}
	return true;
}

bool IAudioStream::write_audio_frame(AVFrame *frame)
{
	
	AVPacket pkt;
    int got_output,ret;
    av_init_packet(&pkt);
    pkt.data = NULL;  
    pkt.size = 0;
	ret = avcodec_encode_audio2(this->audio_codec_context, &pkt, frame, &got_output);

	if (ret < 0) 
	{
        printf( "Error encoding audio frame\n");
        return false;
    }
	if(got_output)
	{
		
		//sendPacket((char *)pkt.data,pkt.size);
		av_free_packet(&pkt);

		if(ret<0)
		{
			printf( "Error writing audio frame\n");
			return false;
		}
	}
	else
	{
		printf( "Unknown Error in AUDIO Encoding! But just return true\n");
	}
	return true;
	
}

bool IAudioStream::initAudioStream()
{
	if(!openAudioStream())
	{
		printf("Can't  add audio stream\n");
		return false;
	}
	return true;
}

void IAudioStream::cleanup()
{
	
	if(this->audio_codec_context!=0)
	{
		avcodec_close(this->audio_codec_context);
		av_freep(&this->audio_codec_context);
	}
	if(this->audio_codec!=0)
	{
		av_freep(&this->audio_codec);
		this->audio_codec=0;
	}
	
}

int IAudioStream::getRequestedFrameSize() const
{
	//printf("Debug:Request frameSize:%d\n",this->audio_codec_context->frame_size);
	return this->audio_codec_context->frame_size;
}