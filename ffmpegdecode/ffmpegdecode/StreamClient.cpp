#include "StreamClient.h"
StreamClient::StreamClient()
{
	this->localPort=DEFAULT_PORT;
	this->fmt=0;
	this->oc=0;
	this->audio_st=0;
	this->video_st=0;
	this->audio_codec=0;
	this->video_codec=0;
	avformat_network_init();
	av_register_all() ;
	avcodec_register_all();
}
StreamClient::~StreamClient()
{

}
void StreamClient::setLocalPort(int port)
{
	this->localPort=port;
}


bool StreamClient::addAudioStream()
{
	return true;
}
bool StreamClient::addVideoStream()
{
	AVCodecContext *c;
	this->video_codec = avcodec_find_decoder(CODEC_ID_H264);
	if (!this->video_codec) 
	{
		printf( "video codec not found/n");
		return false;
    }
	this->video_st = avformat_new_stream(this->oc, this->video_codec);
	if (!this->video_st) 
	{
        printf( "Could not allocate stream\n");
        return false;
    }
	this->video_st->id=this->oc->nb_streams-1;
	c=this->video_st->codec;
	avcodec_get_context_defaults3(c, this->video_codec);
	c->codec_id=CODEC_ID_H264;
	c->bit_rate = 3000000;
    c->width = RWIDTH;
    c->height = RHEIGHT;
	c->gop_size=0;
	c->max_b_frames=0;
    c->pix_fmt = PIX_FMT_YUV420P;
	c->me_range = 16;
    c->max_qdiff = 4;
    c->qmin = 10;
    c->qmax = 51;
	c->rc_max_rate=5000000;
	c->rc_buffer_size=200000;
	c->time_base.den = 25;
    c->time_base.num = 1;
	if (avcodec_open2(c, this->video_codec,NULL) < 0) 
	{
        printf( "could not open codec\n");
        return false;
    }
	
	return true;
}
bool StreamClient::startClient()
{
	
	this->oc=avformat_alloc_context();
	if(oc==NULL)
	{
		printf("Try init avformat failed\n");
		return false;
	}
	
	this->fmt = av_guess_format("rtp", NULL, NULL);
	if (!this->fmt)
    {
        printf("Try init RTP format failed\n");
		return false;
    }
	oc->oformat=this->fmt;
	if(!this->addVideoStream())
	{
		printf("Can open add video stream@!\n");
	    return false;
	}
	_snprintf_s(this->oc->filename, sizeof(this->oc->filename), "rtp://%s:%d", LOCALADDRESS, this->localPort);
	if(avformat_open_input(&oc, this->oc->filename,NULL,NULL) != 0)
	{
		printf("Try init network failed\n");
        return false;
    }
	
	AVPacket pkt;
	av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;
	while(true)
	{
		Sleep(10);
		if (av_read_frame(this->oc, &pkt) >= 0) 
		{
			printf("Got a packet\n");
			av_free_packet(&pkt);
		}
		else
		{
			printf("Got nothing\n");
		}
		
	}
	/**/
	/*
	if(!this->addAudioStream())
	{
		printf("Can open add audio stream@!\n");
	    return false;
	}
	av_dump_format(this->oc, 0,this->oc->filename,0);
	
	*/
	return true;
}