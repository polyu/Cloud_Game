#include "StreamClient.h"
StreamClient::StreamClient()
{
	this->localPort=DEFAULT_PORT;
	this->audio_codec=0;
	this->video_codec=0;
	this->frame=0;
	this->intBuf=(char*)malloc(MAXBUFSIZE);
	this->bufSize=0;
	this->lastWidth=0;
	this->lastHeight=0;
	this->img_convert_ctx=0;
	avcodec_register_all();
}
StreamClient::~StreamClient()
{
	if(intBuf!=0)
	{
		free(intBuf);
		intBuf=0;
	}
	WSACleanup();
}
void StreamClient::setLocalPort(int port)
{
	this->localPort=port;
}
bool StreamClient::poolVideoFrame()
{
	bool markerFound=false;
	int retryTime=0;

	while(!markerFound && retryTime++<MAXRETRYTIME )
	{
			if(videoRTPSession.Poll()<0)
			{
				printf("POLL DATA ERROR\n");
				return false;
			}
			videoRTPSession.BeginDataAccess();
			if (videoRTPSession.GotoFirstSource())
			{
				if(videoRTPSession.GotoNextSource())
				{
						RTPPacket *packet;
						while ((packet = videoRTPSession.GetNextPacket()) != 0)
						{
							if(packet->HasMarker())
							{
								markerFound=true;
								printf("This is frame is marker\n");
							}
							else
							{
								printf("No Marker\n");
								
							}
							
							printf("Got sequence number %d from Source %d Length %d\n",packet->GetExtendedSequenceNumber() ,packet->GetSSRC(),packet->GetPayloadLength());
							if(bufSize+packet->GetPayloadLength()>MAXBUFSIZE)
							{
								printf("BUF FULL\n");
								bufSize=0;
							}
							memcpy(this->intBuf+bufSize,packet->GetPayloadData(),packet->GetPayloadLength());
							this->bufSize+=packet->GetPayloadLength();
							videoRTPSession.DeletePacket(packet);
							if(markerFound) 
							{
								printf("BUF SIZE:%d\n",this->bufSize);
									break;
							}
						}
				}
						
			}
			videoRTPSession.EndDataAccess();
	}
	return markerFound;
}
bool StreamClient::decodeVideoFrame(AVFrame **getframe)
{
	
	if(poolVideoFrame())
	{
		int len, got_frame;
		avpkt.data = (uint8_t*)this->intBuf;
		avpkt.size=this->bufSize;
		FILE *f;
		f=fopen("c:/2.dump","w");
		fwrite(intBuf,bufSize,1,f);
		fclose(f);
		len = avcodec_decode_video2(this->video_codec_context, frame, &got_frame, &avpkt);
		this->bufSize=0;
		if(len<0)
		{
			printf("Error happen when decoding\n");
			return false;
		}
		if (got_frame) 
		{
			if(lastHeight!=frame->height || lastWidth!=frame->width)
			{
				removeSwscale();
				if(!setupSwscale())
				{
					printf("Error happen when setting up swscale!\n");
					return false;
				}
			}
			sws_scale(img_convert_ctx, frame->data, frame->linesize,0, RHEIGHT, picture->data, picture->linesize);  
			printf("Got one picture\n");
			*getframe=picture;
			return true;
		}
		else
		{
			//printf("Bad packet! OOP! Maybe next will be good!\n");
			return false;
		}
	}
	else
	{
		//printf("Try to decode frame But no data availabie\n");
		return false;
	}
}
bool StreamClient::openAudioStream()
{
	return true;
}
bool StreamClient::openVideoStream()
{

	this->video_codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!this->video_codec) 
	{
		printf( "video codec not found/n");
		return false;
    }
	this->video_codec_context = avcodec_alloc_context3(this->video_codec);
	av_opt_set(video_codec_context->priv_data, "tune", "zerolatency", 0);
	av_opt_set(video_codec_context->priv_data, "preset","veryfast",0);
	av_opt_set(video_codec_context->priv_data,"intra-refresh","1",0);
	
	if (avcodec_open2(this->video_codec_context, this->video_codec,NULL) < 0) 
	{
        printf( "could not open codec\n");
        return false;
    }
	return true;
}
bool StreamClient::openVideoRTPClient()
{
	RTPSessionParams sessionparams;
	sessionparams.SetUsePollThread(false);
	sessionparams.SetOwnTimestampUnit(1.0/25.0);
	RTPUDPv4TransmissionParams transparams;
	transparams.SetPortbase(this->localPort);
	int status = videoRTPSession.Create(sessionparams,&transparams);
	if (status < 0)
	{
		printf("Error when create session:%s\n",RTPGetErrorString(status));
		return false;
	}
	printf("RTP video client Init\n");
	return true;
}
bool StreamClient::openAudioRTPClient()
{
	
	RTPSessionParams sessionparams;
	sessionparams.SetUsePollThread(false);
	sessionparams.SetOwnTimestampUnit(1.0/8000.0);
	RTPUDPv4TransmissionParams transparams;
	transparams.SetPortbase(this->localPort+VIDEOAUDIOPORTGAP);
	int status = audioRTPSession.Create(sessionparams,&transparams);
	if (status < 0)
	{
		printf("Error when create audio session:%s\n",RTPGetErrorString(status));
		return false;
	}
	printf("RTP audio client Init\n");
	return true;
}
bool StreamClient::initClient()
{
	
	if(WSAStartup(MAKEWORD(2,2),&dat)!=0)
	{
		printf("Fail init socket\n");
		return false;
	}
	if(!this->openVideoStream())
	{
		printf("Can open add video stream@!\n");
	    return false;
	}
	if(!this->openAudioStream())
	{
		printf("Can open add video stream@!\n");
	    return false;
	}
	if(!this->openVideoRTPClient())
	{
		printf("Can't open video rtp stream@!\n");
	    return false;
	}
	if(!this->openAudioRTPClient())
	{
		printf("Can't open audio rtp stream@!\n");
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

bool StreamClient::setupSwscale()
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
void StreamClient::removeSwscale()
{
	if(img_convert_ctx!=NULL)
	{
		sws_freeContext(img_convert_ctx);
		img_convert_ctx=NULL;
	}
	
}
AVFrame *StreamClient::alloc_picture(enum PixelFormat pix_fmt, int width, int height)
{
	AVFrame *picture = avcodec_alloc_frame();
    if (!picture || avpicture_alloc((AVPicture *)picture, pix_fmt, width, height) < 0)
        av_freep(&picture);
    return picture;
}