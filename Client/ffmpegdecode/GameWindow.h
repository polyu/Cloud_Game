#include "stdafx.h"

#include "VideoStreamDecoder.h"
#include "AudioStreamDecoder.h"
#include <SDL.h>
#include "DataTunnel.h"
#include <queue>
#include <process.h>
#include "Controller.h"
using namespace std;
#ifdef main
#undef main
#endif
#define AUDIOPLAYBUFSIZE 51200
//*-**********************SOUND**************
static void getAudioFromBuffer(void *udata, Uint8 *stream, int len);
static void decodeAudioFromQueue();
//***************************SDL***************
static void initSDL();
//********************Network**************
static void initTunnelNetwork();
static void networkThread(void *);
//**********************Decoder**************
static void initDecoder();
//===================Controller===================
static void initController();
//====================Library=================
static void initExternLibrary();
//=====================Video================
static void SDL_VideoDisplayThread(void *);
//============PUBLIC API===========
int startGame();
