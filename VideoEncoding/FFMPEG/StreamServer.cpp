#include "StreamServer.h"
StreamServer::StreamServer()
{
	avcodec_register_all();
	
	this->video_codec=0;
	this->video_codec_context=0;
	this->audio_codec=0;
	this->audio_codec_context=0;
	this->localPort=DEFAULT_LOCALPORT;
	this->remoteAddr=DEFAULT_REMOTEADDRESS;
	this->remotePort=DEFAULT_REMOTEPORT;
}
StreamServer::~StreamServer()
{
}
void StreamServer::setRemoteAddress(string address,int port)
{
	this->remoteAddr=address;
	this->remotePort=port;
}
bool StreamServer::openAudioStream()
{
	this->audio_codec = avcodec_find_encoder(CODEC_ID_SPEEX);
	if (!this->audio_codec) 
	{
		printf( "video codec not found/n");
		return false;
	}
	this->audio_codec_context=avcodec_alloc_context();
	//=============
	AVCodecContext *c=this->audio_codec_context;
	c->sample_fmt  = AV_SAMPLE_FMT_S16;
    c->bit_rate    = 64000;
    c->sample_rate = 32000;
    c->channels    = 1;
	//c->channel_layout=av_get_channel_layout("DL");
	//=========
	if( avcodec_open2(audio_codec_context, this->audio_codec, NULL)<0)
	{
		printf( "could not open audio codec\n");
        return false;
	}
	return true;
	
}
bool StreamServer::write_video_frame(AVFrame *frame)
{
	long encodeVideoPerformanceClock=clock();
	AVPacket pkt;
    int got_output,ret;
    av_init_packet(&pkt);
    pkt.data = NULL;    // packet data will be allocated by the encoder
    pkt.size = 0;
	ret = avcodec_encode_video2(this->video_codec_context, &pkt, frame, &got_output);
    if (ret < 0) 
	{
        printf( "Error encoding video frame\n");
        return false;
    }
	if (got_output) 
	{
		if (video_codec_context->coded_frame->key_frame)
			pkt.flags |= AV_PKT_FLAG_KEY;
		printf("Try sending %d\n",pkt.size);
		char test[10]="ok";
		int ret=RTPSess.SendPacket(test,10,96,false,160);
		av_free_packet(&pkt);
		if(ret<0)
		{
			printf( "Error writing video frame:%s\n",RTPGetErrorString(ret));
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
bool StreamServer::write_audio_frame(AVFrame *frame)
{
	long encodeAudioPerformanceClock=clock();
	AVPacket pkt;
    int got_output,ret;
    av_init_packet(&pkt);
    pkt.data = NULL;    // packet data will be allocated by the encoder
    pkt.size = 0;
	ret = avcodec_encode_audio2(this->audio_codec_context, &pkt, frame, &got_output);
	if (ret < 0) 
	{
        printf( "Error encoding audio frame\n");
        return false;
    }
	if(got_output)
	{
		
		printf("Try sending audio %d\n",pkt.size);
		
		av_free_packet(&pkt);
		if(ret<0)
		{
			printf( "Error writing audio frame:%s\n",RTPGetErrorString(ret));
			return false;
		}
	}
	else
	{
		printf( "Unknown Error in AUDIO Encoding! But just return true\n");
	}
	return true;
}
bool StreamServer::openVideoStream()
{
	
	this->video_codec = avcodec_find_encoder(CODEC_ID_H264);
	if (!this->video_codec) 
	{
		printf( "video codec not found/n");
		return false;
    }
	
	this->video_codec_context=avcodec_alloc_context3(this->video_codec);
	if (!video_codec_context) 
	{
        printf("Could not allocate video codec context\n");
        return false;
    }
	AVCodecContext *c=video_codec_context;
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
	
	if (avcodec_open2(c, this->video_codec,NULL) < 0) 
	{
        printf( "could not open video codec\n");
        return false;
    }

	return true;
}
bool StreamServer::initStreamServer()
{
	g_hMutex = CreateMutex(NULL, FALSE, L"Mutex");
	if (!g_hMutex)  
    {  
        printf("Failed to create mutex\n");
        return false;  
    }  
	if(WSAStartup(MAKEWORD(2,2), &wsaData)!=0)
	{
		printf("Init Socket Failed");
		return false;
	}
	if(!openVideoStream())
	{
		printf("Can' add video stream! \n");
	   return false;
	}
	if(!openAudioStream())
	{
		printf("Can't  add audio stream\n");
		return false;
	}
	if(!openRTPServer())
	{
		printf("Can't open rtp session\n");
		return false;
	}
	return true;
}

void StreamServer::cleanup()
{
	
	if(this->video_codec_context!=0)
	{
		avcodec_close(video_codec_context);
		av_free(video_codec_context);
		video_codec_context=0;
	}
	if(this->audio_codec_context!=0)
	{
		avcodec_close(audio_codec_context);
		av_free(audio_codec_context);
		audio_codec_context=0;
	}
}
bool StreamServer::openRTPServer()
{
	uint32_t destip=inet_addr(this->remoteAddr.c_str());
	if (destip == INADDR_NONE)
	{
		printf("Bad IP Setting\n");
		return false;
	}
	
	destip = ntohl(destip);
	RTPUDPv4TransmissionParams transparams;
	RTPSessionParams sessparams;
	sessparams.SetOwnTimestampUnit(1.0/25.0);
	transparams.SetPortbase(this->localPort);
	//RTPSess.SetDefaultPayloadType(96);
	//RTPSess.SetDefaultMark(false);
	//RTPSess.SetDefaultTimestampIncrement(160);
	int status = RTPSess.Create(sessparams,&transparams);	
	if(status<0)
	{
		printf("Some error happened!When open RTP SESSION:%s\n",RTPGetErrorString(status));
		return false;
	}
	RTPIPv4Address addr(destip,this->remotePort);
	status = RTPSess.AddDestination(addr);
	if(status<0)
	{
		printf("Some error happened!When open RTP REMOTE END\n");
		return false;
	}
	return true;
}
void StreamServer::setLocalPort(int port)
{
	this->localPort=port;
}