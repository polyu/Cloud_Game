#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <time.h>
#include "StreamServer.h"
#define RESERVEDMEMORY 256
#define SHAREDMEMSIZE 1440*900*32+RESERVEDMEMORY
#define MAXFPS 200
#define RHEIGHT 480
#define RWIDTH 640
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
	void setStreamServer(StreamServer * streamServer);
	void debugEncoder(const char *filename);
private:
BYTE* lpvMem;      // pointer to shared memory
HANDLE hMapObject;
AVCodec *codec;
AVCodecContext *c;
AVFrame *picture;
AVDictionary* opt;
StreamServer * streamServer;
SwsContext *img_convert_ctx; 
uint8_t *outbuf, *picture_buf;
int frameCounter;
int lastWidth,lastHeight;
bool workingThread;
void uninstallSharedMemory();
bool setupSwscale(int in_width,int in_height);
void removeSwscale();
AVFrame* alloc_picture(enum PixelFormat pix_fmt, int width, int height);
bool setupSharedMemory();
bool isMemoryReadable();
void setMemoryWritable();
bool initVideoCodec();
void removeVideoCodec();

long encodingPerformanceTime;
FILE *f;

};