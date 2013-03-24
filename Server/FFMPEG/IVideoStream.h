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


class IVideoStream
{
public:
	IVideoStream();
	~IVideoStream();
	bool initVideoStream();
	void cleanup();
	bool write_video_frame(AVFrame *frame);
	void setQuality(int width,int height,int bandwidth);
private:
	int bandwidth;
	int output_width;
	int output_height;
    AVCodec *video_codec;
	AVCodecContext *video_codec_context;
	int sendOutFrame(AVPacket *packet);
	bool openVideoStream();
};