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


class IVideoStreamServer
{
public:
	IVideoStreamServer();
	~IVideoStreamServer();
	void setRemoteAddress(string address,int videoport);
	bool initVideoStreamServer();
	void cleanup();
	bool write_video_frame(AVFrame *frame);
private:
	AVOutputFormat *vfmt;
    AVFormatContext *voc;
    AVStream *video_st;
    AVCodec *video_codec;
    double video_pts;
	string remoteAddr;
	int remoteVideoPort;
	bool addVideoStream();

};