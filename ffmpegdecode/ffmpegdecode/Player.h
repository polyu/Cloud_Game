#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <time.h>
#include <queue>
using namespace std;
#define RHEIGHT 480
#define RWIDTH 640
#define INBUF_SIZE 100000
extern "C"
{
#ifdef HAVE_AV_CONFIG_H
#undef HAVE_AV_CONFIG_H
#endif
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
bool decode_frame(pair<char*,int> &data);
bool initDecoder();
