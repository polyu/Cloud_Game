#pragma once
#include "stdafx.h"
#include <audioclient.h>
#include <stdio.h>
#include <avrt.h>
#include <iostream>
#include <fstream>
#include <vector> 
#include <iterator>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include "IAudioStreamServer.h"
extern "C"
{
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
}
class ISoundCapturer
{
public:
	ISoundCapturer();
	~ISoundCapturer();
	void setStreamServer(IAudioStreamServer * streamServer);
	bool initISoundCapturer();
	void startFrameLoop();
	void stopFrameLoop();

private:
	IMMDeviceEnumerator *enumerator;
	IMMDevice *device ;
	IAudioClient *audioClient;
	WAVEFORMATEX *waveFormat;
	IAudioCaptureClient *audioCaptureClient;
	IAudioStreamServer * streamServer;
	SwrContext *swr_ctx;
	bool runFlag;
	bool setupSwscale();
	void removeSwscale();
	int alloc_samples_array_and_data(uint8_t ***data, int *linesize, int nb_channels,
                                    int nb_samples, enum AVSampleFormat sample_fmt, int align);\
	AVFrame *frame;
};