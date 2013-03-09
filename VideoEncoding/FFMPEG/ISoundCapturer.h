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
#include "StreamServer.h"

class ISoundCapturer
{
public:
	ISoundCapturer();
	~ISoundCapturer();
	void setStreamServer(StreamServer * streamServer);
	bool initISoundCapturer();
	void startFrameLoop();
	void stopFrameLoop();
private:
	IMMDeviceEnumerator *enumerator;
	IMMDevice *device ;
	IAudioClient *audioClient;
	WAVEFORMATEX *waveFormat;
	IAudioCaptureClient *audioCaptureClient;
	StreamServer * streamServer;
	AVFrame *frame;
	bool runFlag;
};