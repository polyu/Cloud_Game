#pragma once
#include "stdafx.h"
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include "rtpsession.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtpsessionparams.h"
#include "rtperrors.h"

using namespace std;
using namespace jrtplib;
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


class StreamServer
{
public:
	StreamServer();
	~StreamServer();
	void setRemoteAddress(string address,int port);
	void setLocalPort(int port);
	bool initStreamServer();
	void cleanup();
	bool write_video_frame(AVFrame *frame);
	bool write_audio_frame(AVFrame *frame);
	
private:
    AVCodec *audio_codec, *video_codec;
	AVCodecContext *audio_codec_context,*video_codec_context;
	string remoteAddr;
	int remotePort;
	int localPort;
	HANDLE g_hMutex;
	bool openVideoStream();
	bool openAudioStream();
	bool openRTPServer();
	RTPSession RTPSess;
	WSADATA wsaData;
	
};