#include "IVideoStreamServer.h"
IVideoStreamServer::IVideoStreamServer()
{
	this->vfmt=0;
	this->voc=0;
	this->video_st=0;
	this->video_codec=0;
	this->video_pts=0;
	this->remoteAddr=DEFAULT_REMOTEADDRESS;
	this->remoteVideoPort=DEFAULT_RTPVIDEOPORT;
	avformat_network_init();
	av_register_all() ;
	avcodec_register_all();
}
IVideoStreamServer::~IVideoStreamServer()
{
	cleanup();
	WSACleanup();
}
void IVideoStreamServer::setRemoteAddress(string address,int videoport)
{
	this->remoteAddr=address;
	this->remoteVideoPort=videoport;
}

bool IVideoStreamServer::write_video_frame(AVFrame *frame)
{
	//long encodeVideoPerformanceClock=clock();
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
		ret = av_write_frame(voc, &pkt);
		av_free_packet(&pkt);
		if(ret<0)
		{
			printf( "Error writing video frame\n");
			return false;
		}
    }
	else
	{
		printf( "Unknown Error in Encoding! But just return true\n");
	}
	//printf("Encoding Performance %d ms\n",clock()-encodeVideoPerformanceClock);
	return true;
}

bool IVideoStreamServer::addVideoStream()
{
	AVCodecContext *c;
	this->video_codec = avcodec_find_encoder(CODEC_ID_H264);
	if (!this->video_codec) 
	{
		printf( "video codec not found/n");
		return false;
    }
	this->video_st = avformat_new_stream(this->voc, this->video_codec);
	if (!this->video_st) 
	{
        printf( "Could not allocate stream\n");
        return false;
    }
	this->video_st->id=this->voc->nb_streams-1;

	c=this->video_st->codec;
	avcodec_get_context_defaults3(c, this->video_codec);
	c->codec_id=CODEC_ID_H264;
	c->bit_rate = 4000000;
    c->width = RWIDTH;
    c->height = RHEIGHT;
	c->gop_size=0;
	c->max_b_frames=0;
    c->pix_fmt = PIX_FMT_YUV420P;
	c->me_range = 16;
    c->max_qdiff = 4;
    c->qmin = 10;
    c->qmax = 30;
    c->qcompress = (0.6f);
	c->refs=1;
	c->dia_size=1;
	c->keyint_min=46;
	c->active_thread_type= FF_THREAD_SLICE;
	c->thread_type=FF_THREAD_SLICE;
	c->thread_count=4;
	c->slices=4;
	c->rc_max_rate=6500000;
	c->rc_buffer_size=260000;
	c->time_base.den = 25;
    c->time_base.num = 1;
	av_opt_set(c->priv_data, "tune", "zerolatency", 0);
	av_opt_set(c->priv_data, "preset","faster",0);
	av_opt_set(c->priv_data,"intra-refresh","1",0);
	if (avcodec_open2(c, this->video_codec,NULL) < 0) 
	{
        printf( "could not open video codec\n");
        return false;
    }
	_snprintf_s(this->voc->filename, sizeof(this->voc->filename), "rtp://%s:%d", this->remoteAddr.c_str(), this->remoteVideoPort);
	if (avio_open(&(this->voc->pb), this->voc->filename, AVIO_FLAG_WRITE ) < 0)
    {
       printf("Can open video rtp stream! Network Problem\n");
	   return false;
    }
	return true;
}
bool IVideoStreamServer::initVideoStreamServer()
{

	
	this->voc=avformat_alloc_context();
	if(voc==NULL )
	{
		printf("Try init avformat failed\n");
		return false;
	}
	this->vfmt = av_guess_format("rtp", NULL, NULL);
	

	if (!this->vfmt)
    {
        printf("Try init RTP format failed\n");
		return false;
    }

	voc->oformat=this->vfmt;
	

	if(!addVideoStream())
	{
		printf("Can' add video stream! \n");
	   return false;
	}

	if(avformat_write_header(voc, NULL)<0)
	{
		printf("Can not send video header\n");
		return false;
	}

	return true;
}

void IVideoStreamServer::cleanup()
{
	
	av_write_trailer(voc);
	if(this->video_st->codec!=NULL)
		avcodec_close(this->video_st->codec);
	for (int i = 0; i <  this->voc->nb_streams; i++) {
        av_freep(&voc->streams[i]->codec);
        av_freep(&voc->streams[i]);
    }
	avio_close(voc->pb);
	av_free(voc);
}