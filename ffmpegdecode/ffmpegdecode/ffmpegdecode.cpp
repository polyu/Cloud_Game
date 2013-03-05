// ffmpegdecode.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <time.h>
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
#include <libswscale/swscale.h>
}
static void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize,
                     char *filename)
{
    FILE *f;
    int i;

    f=fopen(filename,"w");
    fprintf(f,"P5\n%d %d\n%d\n",xsize,ysize,255);
    for(i=0;i<ysize;i++)
        fwrite(buf + i * wrap,1,xsize,f);
    fclose(f);
}

static void video_decode_example(const char *outfilename, const char *filename)
{
    AVCodec *codec;
    AVCodecContext *c= NULL;
    int frame, got_picture, len;
    FILE *f;
    AVFrame *picture;
    uint8_t inbuf[INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE];
    char buf[1024];
    AVPacket avpkt;

    av_init_packet(&avpkt);

    /* set end of buffer to 0 (this ensures that no overreading happens for damaged mpeg streams) */
    memset(inbuf + INBUF_SIZE, 0, FF_INPUT_BUFFER_PADDING_SIZE);

    printf("Video decoding\n");

    /* find the mpeg1 video decoder */
    codec = avcodec_find_decoder(CODEC_ID_H264);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        exit(1);
    }

    c= avcodec_alloc_context();
	
	c->bit_rate = 300000;
	 c->pix_fmt = PIX_FMT_YUV420P;
    /* resolution must be a multiple of two */
    c->width = RWIDTH;
    c->height = RHEIGHT;
	c->coded_width=RWIDTH;
	c->coded_height=RHEIGHT;
	c->thread_count=4;
	c->slices=4;
	c->time_base.num = 1; 
	c->time_base.den = 25;
    picture= avcodec_alloc_frame();

    

    /* For some codecs, such as msmpeg4 and mpeg4, width and height
       MUST be initialized there because this information is not
       available in the bitstream. */

    /* open it */
    if (avcodec_open(c, codec) < 0) {
        fprintf(stderr, "could not open codec\n");
        exit(1);
    }

    /* the codec gives us the frame size, in samples */

    f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "could not open %s\n", filename);
        exit(1);
    }

    frame = 0;
    for(;;) {
        avpkt.size = fread(inbuf, 1, INBUF_SIZE, f);
        if (avpkt.size == 0)
            break;
		/*if (av_read_frame(c, &avpkt) < 0)
		{
			continue;
		}*/
        /* NOTE1: some codecs are stream based (mpegvideo, mpegaudio)
           and this is the only method to use them because you cannot
           know the compressed data size before analysing it.

           BUT some other codecs (msmpeg4, mpeg4) are inherently frame
           based, so you must call them with all the data for one
           frame exactly. You must also initialize 'width' and
           'height' before initializing them. */

        /* NOTE2: some codecs allow the raw parameters (frame size,
           sample rate) to be changed at any frame. We handle this, so
           you should also take care of it */

        /* here, we use a stream based decoder (mpeg1video), so we
           feed decoder and see if it could decode a frame */
        avpkt.data = inbuf;
        while (avpkt.size > 0) {
            len = avcodec_decode_video2(c, picture, &got_picture, &avpkt);
            if (len < 0) {
                fprintf(stderr, "Error while decoding frame %d\n", frame);
                exit(1);
            }
            if (got_picture) {
                printf("saving frame %3d\n", frame);
                fflush(stdout);

                /* the picture is allocated by the decoder. no need to
                   free it */
                _snprintf(buf, sizeof(buf), outfilename, frame);
                pgm_save(picture->data[0], picture->linesize[0],
                         c->width, c->height, buf);
                frame++;
            }
            avpkt.size -= len;
            avpkt.data += len;
        }
    }

    /* some codecs, such as MPEG, transmit the I and P frame with a
       latency of one frame. You must do the following to have a
       chance to get the last frame of the video */
    avpkt.data = NULL;
    avpkt.size = 0;
    len = avcodec_decode_video2(c, picture, &got_picture, &avpkt);
    if (got_picture) {
        printf("saving last frame %3d\n", frame);
        fflush(stdout);

        /* the picture is allocated by the decoder. no need to
           free it */
        _snprintf(buf, sizeof(buf), outfilename, frame);
        pgm_save(picture->data[0], picture->linesize[0],
                 c->width, c->height, buf);
        frame++;
    }

    fclose(f);

    avcodec_close(c);
    av_free(c);
    av_free(picture);
    printf("\n");
}

int main(int argc, char **argv)
{
    
    avcodec_register_all();

    //    audio_decode_example("/tmp/test.sw", filename);
	video_decode_example("c:/pgm/test%d.pgm", "c:/test.264");

    return 0;
}

