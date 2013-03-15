#pragma once
#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <string>
using namespace std;
extern "C"
{
#ifdef HAVE_AV_CONFIG_H
#undef HAVE_AV_CONFIG_H
#endif
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
}

class IAudioStreamServer
{
public:
	IAudioStreamServer();
	~IAudioStreamServer();
	void setRemoteAddress(string address,int audioport);
	bool initAudioStreamServer();
	void cleanup();
	bool write_audio_frame(AVFrame *frame);

private:
    
    AVCodec *audio_codec;
	AVCodecContext *audio_codec_context;
	string remoteAddr;
	int remoteAudioPort;
	bool addAudioStream();
	
	WSADATA wsaData;
	sockaddr_in local;
	int localport;
	SOCKET sock_fd;
	sockaddr_in remote; 
	bool sendPacket(char* buf, int size);
	BYTE* soundBuffer;
	int soundBufferCursor;
	

};