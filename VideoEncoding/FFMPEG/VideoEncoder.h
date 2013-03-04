#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <string.h>
#define RESERVEDMEMORY 256
#define SHAREDMEMSIZE 1440*900*32+RESERVEDMEMORY
#define MAXFPS 30
extern "C"
{
#ifdef HAVE_AV_CONFIG_H
#undef HAVE_AV_CONFIG_H
#endif
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>
#include <libswscale/swscale.h>
}

class VideoEncoder
{
public:
	VideoEncoder();
	~VideoEncoder();
	bool initEncoder();
	void encodeFrameLoop();
	void stopEncode();
	void debugEncoder(const char *filename);
private:
BYTE* lpvMem;      // pointer to shared memory
HANDLE hMapObject;
AVCodec *codec;
AVCodecContext *c;
AVFrame *picture;
AVDictionary* opt;
uint8_t *outbuf, *picture_buf;
bool workingThread;
void uninstallSharedMemory();
bool setupSharedMemory();
bool isMemoryReadable();
void setMemoryWritable();
bool initVideoCodec();
};