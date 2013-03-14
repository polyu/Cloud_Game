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
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libswscale/swscale.h>
}
bool decode_frame(pair<char*,int> &data,AVFrame** getFrame);
bool initDecoder();
static bool setupSwscale();
static void removeSwscale();
static AVFrame *alloc_picture(enum PixelFormat pix_fmt, int width, int height);