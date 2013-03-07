#pragma once
#include <stdlib.h>
#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <time.h>
#include <queue>
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
class StreamClient
{
public:
	StreamClient();
	~StreamClient();
	void setLocalPort(int port);
	bool startClient();
private:
	int localPort;
	AVOutputFormat *fmt;
    AVFormatContext *oc;
	AVStream *audio_st,*video_st;
    AVCodec *audio_codec, *video_codec;

	bool addVideoStream();
	bool addAudioStream();
};