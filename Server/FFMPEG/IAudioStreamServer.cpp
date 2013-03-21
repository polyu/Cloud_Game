#include "IAudioStreamServer.h"
IAudioStreamServer::IAudioStreamServer()
{
	this->sock_fd=INVALID_SOCKET;
	this->audio_codec=0;
	this->audio_codec_context=0;
	//==========Default Setting==============
	this->remoteAddr=DEFAULT_REMOTEADDRESS;
	this->remoteAudioPort=DEFAULT_RTPAUDIOPORT;
	remote.sin_family = AF_INET;  
    remote.sin_port = htons(this->remoteAudioPort); 
	remote.sin_addr.s_addr = inet_addr(this->remoteAddr.c_str());   
	//=========================
	soundBuffer=(BYTE*)malloc(SOUNDCAPTUREMAXBUFSIZE);
	soundBufferCursor=0;
	
}
IAudioStreamServer::~IAudioStreamServer()
{
	free(soundBuffer);
	if(sock_fd!=INVALID_SOCKET)
		closesocket(sock_fd);
	cleanup();
}
void IAudioStreamServer::setRemoteAddress(string address,int audioport)
{
	this->remoteAddr=address;
	this->remoteAudioPort=audioport;
	remote.sin_family = AF_INET;  
    remote.sin_port = htons(this->remoteAudioPort); 
	remote.sin_addr.s_addr = inet_addr(this->remoteAddr.c_str());   
}
bool IAudioStreamServer::openAudioStream()
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

bool IAudioStreamServer::write_audio_frame(AVFrame *frame)
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
		
		sendPacket((char *)pkt.data,pkt.size);
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
bool IAudioStreamServer::openUDPStream()
{

	
	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock_fd== INVALID_SOCKET)
	{
		printf("Create socket failed\n");
		return false;
	}

	return true;
}
bool IAudioStreamServer::initAudioStreamServer()
{

	if(!openUDPStream())
	{
		printf("Can't open udp audio stream\n");
		return false;
	}
	if(!openAudioStream())
	{
		printf("Can't  add audio stream\n");
		return false;
	}
	
	return true;
}

void IAudioStreamServer::cleanup()
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
bool IAudioStreamServer::sendPacket(char* buf, int size)
{
	int sendSize=sendto(sock_fd, buf, size, 0, (struct sockaddr *)&remote, sizeof(remote));
	if( sendSize!= SOCKET_ERROR)
	{
		printf("Send pieces packet in data %d<->%d\n",sendSize,size);
		return true;
	}
	else
	{
		printf("Send data failed:%d\n",WSAGetLastError());
		return false;
	}
}
int IAudioStreamServer::getRequestedFrameSize() const
{
	//printf("Debug:Request frameSize:%d\n",this->audio_codec_context->frame_size);
	return this->audio_codec_context->frame_size;
}