#include "stdafx.h"
#include <BasicUsageEnvironment.hh>
#include <liveMedia.hh>
#include "VideoStreamDecoder.h"
#include "AudioStreamDecoder.h"
#include <SDL.h>
#include <process.h>
#include <queue>
#include <SDL_image.h>
#include "Controller.h"
using namespace std;

//VIDEO FUNCTION
static void afterGetVideoUnit(void *clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);



//AUDIO FUNCTION

static void afterGetAudioUnit(void *clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);
static void decodeAudioQueue();

//SDL FUNCTION
static int SDL_VideoDisplayThread(void *);
static void SafeCleanUp();
static int NetworkRecvThread(void *);
static void initDecoder();
static void initNetwork();
static void initSDL();
static void initControllerNetwork();
static void getAudioFromBuffer(void *udata, Uint8 *stream, int len);