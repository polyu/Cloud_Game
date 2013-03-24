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

class IAudioStream
{
public:
	IAudioStream();
	~IAudioStream();

	bool initAudioStream();
	void cleanup();
	bool write_audio_frame(AVFrame *frame);
	int getRequestedFrameSize() const;
private:
    
    AVCodec *audio_codec;
	AVCodecContext *audio_codec_context;
	bool openAudioStream();
	
	

};