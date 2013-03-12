#include "StreamDecoder.h"
StreamDecoder::StreamDecoder()
{
	
	this->localPort=DEFAULT_PORT;
	this->outputaudioFrame=0;
	this->audio_codec=0;
	this->video_codec=0;
	this->videoframe=0;
	this->videopicture=0;
	this->swr_ctx=0;
	this->audioframe=0;
	this->lastWidth=0;
	this->lastHeight=0;
	this->img_convert_ctx=0;
	avcodec_register_all();

}
StreamDecoder::~StreamDecoder()
{
	if(this->video_codec_context!=0)
	{
		avcodec_close(video_codec_context);
		video_codec=0;
		
	}
	if(this->audio_codec_context!=0)
	{
		avcodec_close(audio_codec_context);
		this->audio_codec=0;
		
	}
	if(this->audio_codec!=0)
	{
		av_freep(this->audio_codec);
		this->audio_codec=0;
	}
	if(this->video_codec!=0)
	{
		av_freep(this->video_codec);
		this->video_codec=0;
	}
	this->removeSwrcale();
	this->removeSwscale();
	if(this->videoframe!=0)
	{
		avcodec_free_frame(&videoframe);
		videoframe=0;
	}
	if(this->audioframe!=0)
	{
		avcodec_free_frame(&audioframe);
		audioframe=0;
	}
	if(this->outputaudioFrame!=0)
	{
		avcodec_free_frame(&outputaudioFrame);
		outputaudioFrame=0;
	}
	if(this->videopicture!=0)
	{
		avcodec_free_frame(&videopicture);
		videopicture=0;
	}
	av_free_packet(&videoavpkt);
	av_free_packet(&audioavpkt);
}
void StreamDecoder::setLocalPort(int port)
{
	this->localPort=port;
}
bool StreamDecoder::decodeAudioFrame(char*indata,int insize,AVFrame **outdata,int *outSize)
{
	int len, got_frame;
	audioavpkt.data = (uint8_t*)indata;
	audioavpkt.size=insize;
	avcodec_get_frame_defaults(audioframe);
	len = avcodec_decode_audio4(this->audio_codec_context, audioframe, &got_frame, &audioavpkt);
	if(len<0)
	{
		printf("Error happen when decoding\n");
		return false;
	}
	if (got_frame) 
	{
		//========DUE TO AAC FLTP========
		uint8_t **src_data=0;
		int src_linesize;
		alloc_samples_array_and_data(&src_data, &src_linesize, 2,audioframe->nb_samples,AV_SAMPLE_FMT_FLTP , 0);
		int audioFrameSize=av_samples_get_buffer_size(NULL, 2,
                                                       audioframe->nb_samples,
                                                       AV_SAMPLE_FMT_FLTP, 1);
		memcpy(src_data[0],audioframe->data[0],audioFrameSize);

		uint8_t **dst_data=0 ;
		int dst_linesize;
		int dst_nb_samples =av_rescale_rnd(swr_get_delay(swr_ctx, 44100)+audioframe->nb_samples, OUTPUTSAMPLERATE, 44100, AV_ROUND_UP);
		alloc_samples_array_and_data(&dst_data, &dst_linesize, 2,dst_nb_samples, AV_SAMPLE_FMT_S16, 0);
		int retFrameCount=swr_convert(swr_ctx, dst_data, dst_nb_samples, (const uint8_t **)src_data,audioframe->nb_samples );
		if(retFrameCount<0)
		{
			printf("Failed to resample\n");
			return false;
		}
		
		int dst_outputsize = av_samples_get_buffer_size(&dst_linesize, 2, retFrameCount,AV_SAMPLE_FMT_S16 , 0);
		printf("%d\n",dst_outputsize);
		
		*outSize=dst_outputsize;
		outputaudioFrame->nb_samples=retFrameCount;
		int ret=avcodec_fill_audio_frame(outputaudioFrame,2,AV_SAMPLE_FMT_S16,dst_data[0],dst_outputsize,1);
		if(ret<0)
		{
				printf("Fill audio frame failed!\n");
				return false;
		}
		*outdata=outputaudioFrame;
		return true;
	}
	else
	{
		printf("OOP!Unknown Error in decoding\n");
		return false;
	}
	return false;
}
bool StreamDecoder::decodeVideoFrame(char*data,int size,AVFrame **getframe)
{
	int len, got_frame;
	videoavpkt.data = (uint8_t*)data;
	videoavpkt.size=size;
	len = avcodec_decode_video2(this->video_codec_context, videoframe, &got_frame, &videoavpkt);
	if(len<0)
	{
		printf("Error happen when decoding\n");
		return false;
	}
	if (got_frame) 
	{
		if(lastHeight!=videoframe->height || lastWidth!=videoframe->width)
		{
			removeSwscale();
			if(!setupSwscale())
			{
				printf("Error happen when setting up swscale!\n");
				return false;
			}
		}
		sws_scale(img_convert_ctx, videoframe->data, videoframe->linesize,0, RHEIGHT, videopicture->data, videopicture->linesize);  
		*getframe=videopicture;
		return true;
	}
	else
	{
		printf("Bad packet! OOP! Maybe next will be good!\n");
		return false;
	}
	return true;
}
bool StreamDecoder::openAudioCodec()
{
	this->audio_codec=avcodec_find_decoder(AV_CODEC_ID_AAC);
	if (!this->audio_codec) 
	{
		printf( "audio codec not found/n");
		return false;
    }
	this->audio_codec_context = avcodec_alloc_context3(this->audio_codec);
	
	if (avcodec_open2(this->audio_codec_context, this->audio_codec,NULL) < 0) 
	{
        printf( "could not open audio codec\n");
        return false;
    }
	if(!this->setupSwrcale())
	{
		printf( "Resampling Scale init Failed\n");
        return false;
	}
	return true;
}
bool StreamDecoder::openVideoCodec()
{

	this->video_codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!this->video_codec) 
	{
		printf( "video codec not found/n");
		return false;
    }
	this->video_codec_context = avcodec_alloc_context3(this->video_codec);
	if (avcodec_open2(this->video_codec_context, this->video_codec,NULL) < 0) 
	{
        printf( "could not open video codec\n");
        return false;
    }
	return true;
}

bool StreamDecoder::initDecorder()
{
	
	if(!this->openVideoCodec())
	{
		printf("Can open add video stream@!\n");
	    return false;
	}
	if(!this->openAudioCodec())
	{
		printf("Can open add video stream@!\n");
	    return false;
	}
	
	videoframe = avcodec_alloc_frame();
	audioframe = avcodec_alloc_frame();
	outputaudioFrame=avcodec_alloc_frame();
    if (!videoframe || !audioframe || !outputaudioFrame) {
        printf("Could not allocate video frame\n");
        return false;
    }
	av_init_packet(&videoavpkt);
	av_init_packet(&audioavpkt);
	videopicture=alloc_picture(PIX_FMT_YUV420P, RWIDTH, RHEIGHT);
	return true;
}

bool StreamDecoder::setupSwscale()
{
	img_convert_ctx = sws_getContext(videoframe->width, videoframe->height, this->video_codec_context->pix_fmt, 
	RWIDTH, RHEIGHT, PIX_FMT_YUV420P, SWS_FAST_BILINEAR, 
	NULL, NULL, NULL);
	if(img_convert_ctx == NULL) { 
	printf( "Cannot initialize the conversion context!\n"); 
	return false; 
	}
	return true;
}
void StreamDecoder::removeSwscale()
{
	if(img_convert_ctx!=NULL)
	{
		sws_freeContext(img_convert_ctx);
		img_convert_ctx=NULL;
	}
	
}
AVFrame *StreamDecoder::alloc_picture(enum PixelFormat pix_fmt, int width, int height)
{
	AVFrame *picture = avcodec_alloc_frame();
    if (!picture || avpicture_alloc((AVPicture *)picture, pix_fmt, width, height) < 0)
        av_freep(&picture);
    return picture;
}

void StreamDecoder::removeSwrcale()
{
	if(this->swr_ctx!=0)
	{
		swr_free(&swr_ctx);
		this->swr_ctx=0;
	}
}
bool StreamDecoder::setupSwrcale()
{
	swr_ctx = swr_alloc();
    if (!swr_ctx) {
        printf( "Could not allocate resampler context\n");
        return false;
    }
	av_opt_set_int(swr_ctx, "in_channel_count",    2, 0);
	av_opt_set_int(swr_ctx, "in_sample_rate",       44100, 0);
    av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt",  AV_SAMPLE_FMT_FLTP, 0);
	av_opt_set_int(swr_ctx, "out_channel_count",    2, 0);
    av_opt_set_int(swr_ctx, "out_sample_rate",       OUTPUTSAMPLERATE, 0);
    av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
	if (( swr_init(swr_ctx)) < 0) {
       printf( "Failed to initialize the resampling context\n");
	   return false;
    }
	return true;
}
int StreamDecoder::alloc_samples_array_and_data(uint8_t ***data, int *linesize, int nb_channels,
                                    int nb_samples, enum AVSampleFormat sample_fmt, int align)
{
    int nb_planes = av_sample_fmt_is_planar(sample_fmt) ? nb_channels : 1;
    *data = (uint8_t **)av_malloc(sizeof(*data) * nb_planes);
    if (!*data)
	{
		printf("Failed alloca sample\n");
        return AVERROR(ENOMEM);
	}
    return av_samples_alloc(*data, linesize, nb_channels,
                            nb_samples, sample_fmt, align);
}