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
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

static bool GetNextFrame(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx,int videoStream, AVFrame *pFrame)
{
   static AVPacket packet;
   static int      bytesRemaining=0;
   static uint8_t  *rawData;
   static bool     fFirstTime=true;
   int             bytesDecoded;
   int             frameFinished;
   // First time we're called, set packet.data to NULL to indicate it
   // doesn't have to be freed
   if (fFirstTime){
       fFirstTime = false;
       packet.data = NULL;
   }
   // Decode packets until we have decoded a complete frame
   while (true)
   {
	   av_read_frame();
       // Work on the current packet until we have decoded all of it
       while (bytesRemaining > 0)
       {
           // Decode the next chunk of data
           bytesDecoded = avcodec_decode_video2(pCodecCtx, pFrame,
               &frameFinished, rawData, bytesRemaining);
           // Was there an error?
           if (bytesDecoded < 0){
               fprintf(stderr, "Error while decoding frame\n");
               return false;
           }
           bytesRemaining -= bytesDecoded;
           rawData += bytesDecoded;
           // Did we finish the current frame? Then we can return
           if (frameFinished)
               return true;
       }
       // Read the next packet, skipping all packets that aren't for this
       // stream
       do{
           // Free old packet
           if(packet.data != NULL)
               av_free_packet(&packet);
           // Read new packet
           if(av_read_packet(pFormatCtx, &packet) < 0)
               goto loop_exit;
       } while(packet.stream_index != videoStream);
       bytesRemaining = packet.size;
       rawData = packet.data;
   }
loop_exit:
   // Decode the rest of the last frame
   bytesDecoded = avcodec_decode_video(pCodecCtx, pFrame, &frameFinished, 
               rawData, bytesRemaining);
   // Free last packet
   if(packet.data != NULL)
       av_free_packet(&packet);
   return frameFinished != 0;
}
int main()
{
   AVFormatContext *pFormatCtx;
   int             i, videoStream;
   AVCodecContext  *pCodecCtx;
   AVCodec         *pCodec;
   AVFrame         *pFrame; 
   AVFrame         *pFrameYUV;
   clock_t         t;
   double          fps;
   int                y_size, i_frame=0;
   int                numBytes;
   uint8_t         *buffer;        
   char* infile="test.264";
   char* outfile="out.yuv";
   FILE* fp=fopen(outfile, "wb");
   if (fp==NULL){
           fprintf(stderr, "\nCan't open file %s!", infile);
           return -1;
    }
   // Register all formats and codecs
   av_register_all();
   // Open video file
   if (av_open_input_file(&pFormatCtx, infile, NULL, 0, NULL) != 0)
       return -1; // Couldn't open file
   // Retrieve stream information
   if (av_find_stream_info(pFormatCtx) < 0)
       return -1; // Couldn't find stream information
   // Dump information about file onto standard error
   dump_format(pFormatCtx, 0, infile, false);
    t = clock();       
   // Find the first video stream
   videoStream = -1;
   for (i=0; i<pFormatCtx->nb_streams; i++)
       if(pFormatCtx->streams[/*此处不隔开，后面的字体全部是斜体*/i]->codec->codec_type == CODEC_TYPE_VIDEO){
           videoStream=i;
           break;
       }
   if (videoStream == -1)
       return -1; // Didn't find a video stream
   // Get a pointer to the codec context for the video stream
   pCodecCtx = pFormatCtx->streams[videoStream]->codec;
   // Find the decoder for the video stream
   pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
   if (pCodec == NULL)
       return -1; // Codec not found
   // Inform the codec that we can handle truncated bitstreams -- i.e.,
   // bitstreams where frame boundaries can fall in the middle of packets
   if(pCodec->capabilities & CODEC_CAP_TRUNCATED)
       pCodecCtx->flags|=CODEC_FLAG_TRUNCATED;
   // Open codec
   if (avcodec_open(pCodecCtx, pCodec) < 0)
       return -1; // Could not open codec
   // Allocate video frame
   pFrame = avcodec_alloc_frame();
  // Allocate an AVFrame structure
   pFrameYUV=avcodec_alloc_frame();
   if(pFrameYUV == NULL)
       return -1;        
   // Determine required buffer size and allocate buffer
   numBytes=avpicture_get_size(PIX_FMT_YUV420P, pCodecCtx->width,
       pCodecCtx->height);
   buffer = (uint8_t*)malloc(numBytes);        
   // Assign appropriate parts of buffer to image planes in pFrameRGB
   avpicture_fill((AVPicture *)pFrameYUV, buffer, PIX_FMT_YUV420P,
       pCodecCtx->width, pCodecCtx->height);
   // Read frames
   while(GetNextFrame(pFormatCtx, pCodecCtx, videoStream, pFrame))
     {        
           img_convert((AVPicture *)pFrameYUV, PIX_FMT_YUV420P, (AVPicture*)pFrame, 
                               pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);                
            i_frame++;
            y_size = pCodecCtx->width * pCodecCtx->height;                
#if 1
           if (i_frame==1) //only output onr time
           {
       printf("\n:lolpFrame->linesize[0]=%d, pFrame->linesize[1]=%d, pFrame->linesize[2]=%d!\n",
                pFrame->linesize[0], pFrame->linesize[1], pFrame->linesize[2]);
       printf("\n:lolpFrameYUV->linesize[0]=%d, pFrameYUV->linesize[1]=%d, pFrameYUV->linesize[2]=%d!",
               pFrameYUV->linesize[0], pFrameYUV->linesize[1], pFrameYUV->linesize[2]);
            }
#endif                
            fwrite(pFrameYUV->data[0], 1, y_size, fp);
            fwrite(pFrameYUV->data[1], 1, (y_size/4), fp);
            fwrite(pFrameYUV->data[2], 1, (y_size/4), fp);
      }
    //calculate decode rate
    fclose(fp);
    t = clock() - t;
    fps = (double)(t) / CLOCKS_PER_SEC;
    fps = i_frame / fps;
    printf("\n==>Decode rate %.4f fps!\n", fps);   
   // Free the YUV image
   free(buffer);
   av_free(pFrameYUV);
   // Free the YUV frame
   av_free(pFrame);
   // Close the codec
   avcodec_close(pCodecCtx);
   // Close the video file
   av_close_input_file(pFormatCtx);
   return 0;
}