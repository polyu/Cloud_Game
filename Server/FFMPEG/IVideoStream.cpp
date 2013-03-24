#include "IVideoStream.h"
IVideoStream::IVideoStream()
{
	this->video_codec_context=0;
	this->video_codec=0;
	this->output_width=RWIDTH;
	this->output_height=RHEIGHT;
	this->bandwidth=RBANDWIDTH;
}
IVideoStream::~IVideoStream()
{
	cleanup();
	
}
void IVideoStream::setQuality(int width,int height,int bandwidth)
{
	this->output_width=width;
	this->output_height=height;
	this->bandwidth=bandwidth;
}
int IVideoStream::sendOutFrame(AVPacket *pkt)
{
	PBYTE p_buffer = pkt->data;
	int	i_buffer = pkt->size;

	int writeSize = 0;

	while( i_buffer > 4 && ( p_buffer[0] != 0 || p_buffer[1] != 0 || p_buffer[2] != 1 ) )
	{
		i_buffer--;
		p_buffer++;
	}
	
	while( i_buffer > 4 )
	{
		int i_offset;
		int i_size = i_buffer;
		int i_skip = i_buffer;

	/* search nal end */
		for( i_offset = 4; i_offset+2 < i_buffer ; i_offset++)
		{
			if( p_buffer[i_offset] == 0 && p_buffer[i_offset+1] == 0 && p_buffer[i_offset+2] == 1 )
			{
				/* we found another startcode */
				i_size = i_offset - ( p_buffer[i_offset-1] == 0 ? 1 : 0);
				i_skip = i_offset;
				break;
			} 
		}
		UINT iWrite;
	/* TODO add STAP-A to remove a lot of overhead with small slice/sei/... */
	//UINT iWrite = TransportH264Nal(p_buffer, i_size, pts, (i_size >= i_buffer) );
		if (iWrite > 0 )
			writeSize += iWrite;

		i_buffer -= i_skip;
		p_buffer += i_skip;
	}
	return writeSize;
}
bool IVideoStream::write_video_frame(AVFrame *frame)
{
	//long encodeVideoPerformanceClock=clock();
	AVCodecContext *c = this->video_codec_context;
	AVPacket pkt;
    int got_output,ret;
    av_init_packet(&pkt);
    pkt.data = NULL;    // packet data will be allocated by the encoder
    pkt.size = 0;
	ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
    if (ret < 0) 
	{
        printf( "Error encoding video frame\n");
        return false;
    }
	if (got_output) 
	{
		
		//ret = av_write_frame(voc, &pkt);
		av_free_packet(&pkt);
		if(ret<0)
		{
			printf( "Error writing video frame\n");
			return false;
		}
    }
	else
	{
		printf( "Unknown Error in Encoding! But just return true\n");
	}
	//printf("Encoding Performance %d ms\n",clock()-encodeVideoPerformanceClock);
	return true;
}


bool IVideoStream::initVideoStream()
{

if(!openVideoStream())
	{
		printf("Can't  add audio stream\n");
		return false;
	}
	return true;
}

bool IVideoStream::openVideoStream()
{
	this->video_codec = avcodec_find_encoder(CODEC_ID_H264);
	if (!this->video_codec) 
	{
		printf( "aideo codec not found/n");
		return false;
    }
	this->video_codec_context=avcodec_alloc_context3(this->video_codec);
	this->video_codec_context->codec_id=CODEC_ID_H264;
	this->video_codec_context->bit_rate = this->bandwidth;
	this->video_codec_context->width = this->output_width;
	this->video_codec_context->height = this->output_height;
	this->video_codec_context->gop_size=0;
	this->video_codec_context->max_b_frames=0;
    this->video_codec_context->pix_fmt = PIX_FMT_YUV420P;
	this->video_codec_context->me_range = 16;
    this->video_codec_context->max_qdiff = 4;
    this->video_codec_context->qmin = 10;
    this->video_codec_context->qmax = 51;
	this->video_codec_context->refs=1;
	this->video_codec_context->dia_size=1;
	this->video_codec_context->keyint_min=46;
	this->video_codec_context->active_thread_type= FF_THREAD_SLICE;
	this->video_codec_context->thread_type=FF_THREAD_SLICE;
	this->video_codec_context->thread_count=4;
	this->video_codec_context->slices=4;
	/*c->rc_max_rate=6500000;
	c->rc_buffer_size=260000;*/
	this->video_codec_context->time_base.den = 25;
    this->video_codec_context->time_base.num = 1;
	av_opt_set(this->video_codec_context->priv_data, "tune", "zerolatency", 0);
	av_opt_set(this->video_codec_context->priv_data, "preset","faster",0);
	av_opt_set(this->video_codec_context->priv_data,"intra-refresh","1",0);
	if( avcodec_open2(this->video_codec_context, this->video_codec, NULL)<0)
	{
		printf( "could not open video codec\n");
        return false;
	}
	return true;
}
void IVideoStream::cleanup()
{

	if(this->video_codec_context!=NULL)
	{
		avcodec_close(this->video_codec_context);
		av_freep(&(this->video_codec_context));
	}
	if(this->video_codec!=NULL)
	{
		av_freep(&video_codec);
	}
	

}