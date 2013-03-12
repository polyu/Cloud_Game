#pragma once

#include <stdlib.h>
#include "stdafx.h"
#include <stdio.h>
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
#include <libswresample/swresample.h>
}


class StreamDecoder
{
public:
	StreamDecoder();
	~StreamDecoder();
	void setLocalPort(int port);
	bool initDecorder();
	bool decodeVideoFrame(char *indata,int insize,AVFrame **frame);
	bool decodeAudioFrame(char *indata,int insize,AVFrame **frame,int *outSize);
private:
	int localPort;
    AVCodec *audio_codec, *video_codec;
	AVCodecContext *audio_codec_context,*video_codec_context;
	bool openVideoCodec();
	bool openAudioCodec();
	bool setupSwscale();
	void removeSwscale();
	AVPacket videoavpkt;
	AVFrame* videoframe;
	AVFrame* videopicture;
	AVFrame *outputaudioFrame;
	AVPacket audioavpkt;
	AVFrame* audioframe;
	SwrContext *swr_ctx;
	int alloc_samples_array_and_data(uint8_t ***data, int *linesize, int nb_channels,
                                    int nb_samples, enum AVSampleFormat sample_fmt, int align);
	bool setupSwrcale();
	void removeSwrcale();
	int lastWidth;
	int lastHeight;
	SwsContext *img_convert_ctx;
	AVFrame* alloc_picture(enum PixelFormat pix_fmt, int width, int height);
	
};