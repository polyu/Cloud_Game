#include "StreamServer.h"
StreamServer::StreamServer()
{
	this->fmt=0;
	this->oc=0;
	this->audio_st=0;
	this->video_st=0;
	this->audio_codec=0;
	this->video_codec=0;
	this->audio_pts=0;
	this->video_pts=0;
	this->remoteAddr=DEFAULT_ADDRESS;
	this->remotePort=DEFAULT_PORT;
	avformat_network_init();
	av_register_all() ;
	avcodec_register_all();
}
StreamServer::~StreamServer()
{
}
void StreamServer::setRemoteAddress(string address,int port)
{
	this->remoteAddr=address;
	this->remotePort=port;
}
bool StreamServer::addAudioStream()
{
	return true;
}
bool StreamServer::write_video_frame(AVFrame *frame)
{
	long encodeVideoPerformanceClock=clock();
	AVCodecContext *c = this->video_st->codec;
	AVPacket pkt;
    int got_output,ret;
    av_init_packet(&pkt);
    pkt.data = NULL;    // packet data will be allocated by the encoder
    pkt.size = 0;
	ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
    if (ret < 0) 
	{
        printf( "Error encoding video frame\n");
        return false;
    }
	if (got_output) 
	{
		if (c->coded_frame->key_frame)
			pkt.flags |= AV_PKT_FLAG_KEY;
		pkt.stream_index = this->video_st->index;
		if(WaitForSingleObject(g_hMutex, 5)==WAIT_OBJECT_0)//Max Wait 5MS
		{
			ret = av_write_frame(oc, &pkt);
		}
		av_free_packet(&pkt);
		if(ret<0)
		{
			printf( "Error writing video frame\n");\
			return false;
		}
    }
	else
	{
		printf( "Unknown Error in Encoding! But just return true\n");
	}
	printf("Encoding Performance %d ms\n",clock()-encodeVideoPerformanceClock);
	return true;
}
bool StreamServer::write_audio_frame(AVFrame *frame)
{
	return true;
}
bool StreamServer::addVideoStream()
{
	AVCodecContext *c;
	this->video_codec = avcodec_find_encoder(CODEC_ID_H264);
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
    c->qcompress = (0.6f);
	c->refs=1;
	c->dia_size=1;
	c->keyint_min=46;
	c->active_thread_type= FF_THREAD_SLICE;
	c->thread_type=FF_THREAD_SLICE;
	c->thread_count=4;
	c->slices=4;
	c->rc_max_rate=5000000;
	c->rc_buffer_size=200000;
	c->time_base.den = 25;
    c->time_base.num = 1;
	av_opt_set(c->priv_data, "tune", "zerolatency", 0);
	av_opt_set(c->priv_data, "preset","veryfast",0);
	av_opt_set(c->priv_data,"intra-refresh","1",0);
	//======================================
	if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;
	//======================================
	return true;
}
bool StreamServer::startStreamServer()
{
	g_hMutex = CreateMutex(NULL, FALSE, L"Mutex");
	if (!g_hMutex)  
    {  
        printf("Failed to create mutex\n");
        return false;  
    }  
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
	if(!addVideoStream())
	{
		printf("Can' add video stream! \n");
	   return false;
	}
	if(!addAudioStream())
	{
		printf("Can't  add audio stream\n");
		return false;
	}
	if(!openVideoContext())
	{
		printf("Can't  open video context\n");
		return false;
	}
	if(!openAudioContext())
	{
		printf("Can't  open audio context\n");
		return false;
	}
	_snprintf_s(this->oc->filename, sizeof(this->oc->filename), "rtp://%s:%d", this->remoteAddr.c_str(), this->remotePort);
	if (avio_open(&(this->oc->pb), this->oc->filename, AVIO_FLAG_WRITE ) < 0)
    {
       printf("Can open rtp stream! Network Problem\n");
	   return false;
    }
	if(avformat_write_header(oc, NULL)<0)
	{
		printf("Can not send header\n");
		return false;
	}
	return true;
}
bool StreamServer::openAudioContext()
{
	return true;
}
bool StreamServer::openVideoContext()
{
	AVCodecContext *c = this->video_st->codec;
	if (avcodec_open2(c, this->video_codec,NULL) < 0) 
	{
        printf( "could not open codec\n");
        return false;
    }
	
	return true;
}
void StreamServer::cleanup()
{
	av_write_trailer(oc);
	if(this->video_st->codec!=NULL)
		avcodec_close(this->video_st->codec);

	if(this->audio_st->codec!=NULL)
		avcodec_close(this->audio_st->codec);

	for (int i = 0; i <  this->oc->nb_streams; i++) {
        av_freep(&oc->streams[i]->codec);
        av_freep(&oc->streams[i]);
    }
	avio_close(oc->pb);
	av_free(oc);
}