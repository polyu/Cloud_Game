#pragma once
#include "rtpsession.h"
#include "rtppacket.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtpsessionparams.h"
#include "rtperrors.h"
#include "rtpmemorymanager.h"
#include <stdlib.h>
#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <time.h>
#include <queue>
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


class StreamClient
{
public:
	StreamClient();
	~StreamClient();
	void setLocalPort(int port);
	bool initClient();
	bool decodeVideoFrame(AVFrame **frame);
	
private:
	int localPort;
    AVCodec *audio_codec, *video_codec;
	AVCodecContext *audio_codec_context,*video_codec_context;
	bool openVideoStream();
	bool openAudioStream();
	bool openVideoRTPClient();
	bool openAudioRTPClient();
	bool poolVideoFrame();
	bool setupSwscale();
	void removeSwscale();
	WSADATA dat;
	RTPSession videoRTPSession;
	RTPSession audioRTPSession;
	char *intBuf;
	int bufSize;
	AVPacket avpkt;
	AVFrame* frame;
	AVFrame* picture;
	int lastWidth;
	int lastHeight;
	SwsContext *img_convert_ctx;
	AVFrame* alloc_picture(enum PixelFormat pix_fmt, int width, int height);
};