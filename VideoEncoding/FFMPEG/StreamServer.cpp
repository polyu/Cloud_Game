#include "StreamServer.h"
StreamServer::StreamServer()
{
	this->vfmt=0;
	this->afmt=0;
	this->voc=0;
	this->aoc=0;
	this->audio_st=0;
	this->video_st=0;
	this->audio_codec=0;
	this->video_codec=0;
	this->audio_pts=0;
	this->video_pts=0;
	this->remoteAddr=DEFAULT_REMOTEADDRESS;
	this->remoteVideoPort=DEFAULT_REMOTEPORT;
	this->remoteAudioPort=DEFAULT_REMOTEPORT+VIDEOAUDIOPORTGAP;
	avformat_network_init();
	av_register_all() ;
	avcodec_register_all();
}
StreamServer::~StreamServer()
{
	WSACleanup();
}
void StreamServer::setRemoteAddress(string address,int videoport,int audioport)
{
	this->remoteAddr=address;
	this->remoteVideoPort=videoport;
	this->remoteAudioPort=audioport;
}
bool StreamServer::addAudioStream()
{
	AVCodecContext *c;
	this->audio_codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
	if (!this->audio_codec) 
	{
		printf( "aideo codec not found\n");
		return false;
    }
	this->audio_st = avformat_new_stream(this->aoc, this->audio_codec);
	if (!this->audio_st) 
	{
        printf( "Could not allocate stream\n");
        return false;
    }
	this->audio_st->id=this->aoc->nb_streams-1;
	c=this->audio_st->codec;
	c->sample_fmt  = AV_SAMPLE_FMT_S16;
    c->bit_rate    = 128000;
    c->sample_rate = OUTPUTSAMPLERATE;
    c->channels    = 2;
	c->time_base.num= 1;
	c->time_base.den= OUTPUTSAMPLERATE;
	c->channel_layout=3;
	//c->channel_layout=av_get_channel_layout("DL");
	//======================================
	/*if (aoc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;*/
	//======================================
	if( avcodec_open2(c, this->audio_codec, NULL)<0)
	{
		printf( "could not open audio codec\n");
        return false;
	}
	
	_snprintf_s(this->aoc->filename, sizeof(this->aoc->filename), "rtp://%s:%d", this->remoteAddr.c_str(), this->remoteAudioPort);
	if (avio_open(&(this->aoc->pb), this->aoc->filename, AVIO_FLAG_WRITE ) < 0)
    {
       printf("Can open audio rtp stream! Network Problem\n");
	   return false;
    }

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
bool StreamServer::write_audio_frame(AVFrame *frame)
{
	
	long encodeAudioPerformanceClock=clock();
	AVCodecContext *c = this->audio_st->codec;
	AVPacket pkt;
    int got_output,ret;
    av_init_packet(&pkt);
    pkt.data = NULL;    // packet data will be allocated by the encoder
    pkt.size = 0;
	ret = avcodec_encode_audio2(c, &pkt, frame, &got_output);
	
	//printf("LineSize:%d\n",frame->linesize[0]);
	if (ret < 0) 
	{
        printf( "Error encoding audio frame\n");
        return false;
    }
	if(got_output)
	{
		/*FILE *f2=fopen("c:/12345.mp3","a");
		printf("writng 04 %d\n",pkt.size,pkt);
		fwrite(pkt.data,pkt.size,1,f2);
		fclose(f2);*/
		//==========Debug
		/*AVFrame *dframe=avcodec_alloc_frame();
		avcodec_get_frame_defaults(dframe);
		AVCodec *decoder=avcodec_find_decoder(AV_CODEC_ID_AAC);
		AVCodecContext *decodecontext=avcodec_alloc_context3(decoder);
		avcodec_open2(decodecontext, decoder,NULL);
		int pic;
		int ret=avcodec_decode_audio4(decodecontext,dframe,&pic,&pkt);
		if(ret<0)
		{
			printf("Seem fuck\n");
		}
		printf("LINE SIZE:%d<->%d\n",dframe->linesize[0],dframe->nb_samples);
		FILE *f=fopen("c:/o2.dump","w");
		fwrite(dframe->data[0],dframe->linesize[0],1,f);
				fclose(f);*/
		//==========Debug
		pkt.stream_index = this->audio_st->index;
		ret = av_write_frame(aoc, &pkt);
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
bool StreamServer::addVideoStream()
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
	av_opt_set(c->priv_data, "preset","faster",0);
	av_opt_set(c->priv_data,"intra-refresh","1",0);
	//======================================
	/*if (voc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;*/
	//======================================
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
bool StreamServer::initStreamServer()
{

	g_hMutex = CreateMutex(NULL, FALSE, L"Mutex");
	if (!g_hMutex)  
    {  
        printf("Failed to create mutex\n");
        return false;  
    }  
	this->aoc=avformat_alloc_context();
	this->voc=avformat_alloc_context();
	if(voc==NULL || aoc==NULL )
	{
		printf("Try init avformat failed\n");
		return false;
	}
	this->vfmt = av_guess_format("rtp", NULL, NULL);
	this->afmt = av_guess_format("rtp", NULL, NULL);
	
	if (!this->vfmt || !this->afmt )
    {
        printf("Try init RTP format failed\n");
		return false;
    }
	
	voc->oformat=this->vfmt;
	aoc->oformat=this->afmt;

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

	/*if(!generateSDP())
	{
		printf("SDP Failed\n");
		return false;
	}*/
	
	if(avformat_write_header(aoc, NULL)<0)
	{
		printf("Can not send audio header\n");
		return false;
	}
	if(avformat_write_header(voc, NULL)<0)
	{
		printf("Can not send video header\n");
		return false;
	}

	return true;
}
bool StreamServer::generateSDP()
{
	AVFormatContext* ac[2]={this->voc,this->aoc};
	if(av_sdp_create(ac,2,sdpBuf,20480)<0)
	{
		printf("Can not generate SDP header\n");
		return false;
	}
	printf("SDP:%s\n",sdpBuf);
	/*//=====For Debug===========
	FILE *f;
	f=fopen("c:/1.sdp","w");
	fwrite(sdpBuf,1024,1,f);
	fclose(f);
	//=================*/
	return true;
}
void StreamServer::cleanup()
{
	av_write_trailer(aoc);
	av_write_trailer(voc);
	if(this->video_st->codec!=NULL)
		avcodec_close(this->video_st->codec);

	if(this->audio_st->codec!=NULL)
		avcodec_close(this->audio_st->codec);

	for (int i = 0; i <  this->aoc->nb_streams; i++) {
        av_freep(&aoc->streams[i]->codec);
        av_freep(&aoc->streams[i]);
    }
	avio_close(aoc->pb);
	av_free(aoc);

	for (int i = 0; i <  this->voc->nb_streams; i++) {
        av_freep(&voc->streams[i]->codec);
        av_freep(&voc->streams[i]);
    }
	avio_close(voc->pb);
	av_free(voc);
}