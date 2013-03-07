#pragma once
#include "stdafx.h"
#include <windows.h>
#include <audioclient.h>
#include <stdio.h>
#include <avrt.h>
#include <iostream>
#include <fstream>
#include <vector> 
#include <iterator>
#include <mmsystem.h>
#include <mmdeviceapi.h>


class SoundEncoder
{
public:
	SoundEncoder();
	~SoundEncoder();
	bool initSoundCapture();
	bool initSoundEncoder();
	void startFrameLoop();
	void stopFrameLoop();
private:
	IMMDeviceEnumerator *enumerator;
	IMMDevice *device ;
	IAudioClient *audioClient;
	WAVEFORMATEX *waveFormat;
	IAudioCaptureClient *audioCaptureClient;
	bool runFlag;
};