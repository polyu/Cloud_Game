#include "ISoundCapturer.h"
ISoundCapturer::ISoundCapturer()
{
	this->device=0;
	this->enumerator=0;
	this->audioClient=0;
	this->waveFormat=0;
	this->audioCaptureClient=0;
	this->streamServer=0;
	this->frame=0;
	this->swr_ctx=0;
	runFlag=false;
}
ISoundCapturer::~ISoundCapturer()
{
	if(waveFormat!=0)
	{
		CoTaskMemFree(waveFormat);
	}
	removeSwscale();
}
void ISoundCapturer::removeSwscale()
{
	if(this->swr_ctx!=0)
	{
		swr_free(&swr_ctx);
		this->swr_ctx=0;
	}
}
bool ISoundCapturer::setupSwscale()
{
	swr_ctx = swr_alloc();
    if (!swr_ctx) {
        printf( "Could not allocate resampler context\n");
        return false;
    }
	av_opt_set_int(swr_ctx, "in_channel_count",    waveFormat->nChannels, 0);
	av_opt_set_int(swr_ctx, "in_sample_rate",       waveFormat->nSamplesPerSec, 0);
    av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt",  AV_SAMPLE_FMT_S16, 0);
	av_opt_set_int(swr_ctx, "out_channel_count",    2, 0);
    av_opt_set_int(swr_ctx, "out_sample_rate",       OUTPUTSAMPLERATE, 0);
    av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
	if (( swr_init(swr_ctx)) < 0) {
       printf( "Failed to initialize the resampling context\n");
	   return false;
    }
	return true;
}
void ISoundCapturer::startFrameLoop()
{
	
	HRESULT hr;
	long debugFrameCounter=0;
	while(runFlag)
	{
		Sleep(ANTISPIN);
		debugFrameCounter++;
		UINT32 nextPacketSize;
		hr = audioCaptureClient->GetNextPacketSize(&nextPacketSize);
		if (FAILED(hr)) 
		{
			  printf("IAudioCaptureClient::GetNextPacketSize failed on pass %u: hr = 0x%08x\n", debugFrameCounter, hr);
			  audioClient->Stop();
			  audioCaptureClient->Release();
			  audioClient->Release(); 
			  runFlag=false;
			  return;
		}

		if (nextPacketSize == 0) 
		{ // no data yet
		  continue;
		}
		//printf("Next packet Size %d\n",nextPacketSize);
    // get the captured data
		BYTE *data;
		UINT32 frameCount;
		DWORD bufferFlags;

		hr = audioCaptureClient->GetBuffer(&data, &frameCount, &bufferFlags, NULL, NULL);
		if (FAILED(hr)) 
		{
		  printf("IAudioCaptureClient::GetBuffer failed on pass %u: hr = 0x%08x\n", debugFrameCounter, hr);
		  audioClient->Stop();
		  audioCaptureClient->Release();
		  audioClient->Release();   
		  runFlag=false;
		  return;            
		}

		if (bufferFlags & AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY) {
		  printf("IAudioCaptureClient::GetBuffer reports 'data discontinuity' on pass %u\n", debugFrameCounter);
		  audioCaptureClient->ReleaseBuffer(frameCount);
		  continue;
		}
		if (bufferFlags & AUDCLNT_BUFFERFLAGS_SILENT) {
		  printf("IAudioCaptureClient::GetBuffer reports 'silent' on pass %u\n", debugFrameCounter);
		  audioCaptureClient->ReleaseBuffer(frameCount);
		  continue;
		}
		if (bufferFlags & AUDCLNT_BUFFERFLAGS_TIMESTAMP_ERROR) {
		  printf("IAudioCaptureClient::GetBuffer reports 'timestamp error' on pass %u\n", debugFrameCounter);
		  audioCaptureClient->ReleaseBuffer(frameCount);
		  continue;
		}

		if (frameCount == 0) {
		  printf("IAudioCaptureClient::GetBuffer said to read 0 frames on pass %u\n", debugFrameCounter);
		  audioClient->Stop();
		  audioCaptureClient->Release();
		  audioClient->Release();
		  runFlag=false;
		  return;            
		}
		int bytesToWrite = frameCount * waveFormat->nBlockAlign;
		int ret=-1;
		
		//===========Encode data and byteToWrite===========
		if(this->waveFormat->nChannels==2&&this->waveFormat->nSamplesPerSec==OUTPUTSAMPLERATE)//Not need to cumsom cpu
		{
			frame->nb_samples=frameCount;
			ret=avcodec_fill_audio_frame(frame,2,AV_SAMPLE_FMT_S16,data,bytesToWrite,1);
			if(ret<0)
			{
				printf("Fill audio frame failed!\n");
			}
			this->streamServer->write_audio_frame(frame);
		}
		else
		{
			uint8_t **src_data=0;
			int src_linesize;
			uint8_t **dst_data=0 ;
			int dst_linesize;
			int dst_nb_samples =av_rescale_rnd(swr_get_delay(swr_ctx, waveFormat->nSamplesPerSec)+frameCount, OUTPUTSAMPLERATE, waveFormat->nSamplesPerSec, AV_ROUND_UP);
			alloc_samples_array_and_data(&src_data, &src_linesize, 2,frameCount, AV_SAMPLE_FMT_S16, 0);
			alloc_samples_array_and_data(&dst_data, &dst_linesize, 2,dst_nb_samples, AV_SAMPLE_FMT_S16, 0);
			memcpy(src_data[0],data,bytesToWrite);
			int retFrameCount=swr_convert(swr_ctx, dst_data, dst_nb_samples, (const uint8_t **)src_data, frameCount);
			if(retFrameCount<0)
			{
				printf("resample failed\n");
				continue;
			}
			int dst_outputsize = av_samples_get_buffer_size(&dst_linesize, 2, retFrameCount,AV_SAMPLE_FMT_S16 , 0);
			printf("Convert frame %d<---->%d\n",retFrameCount,dst_outputsize);
			frame->nb_samples=retFrameCount;
			ret=avcodec_fill_audio_frame(frame,2,AV_SAMPLE_FMT_S16,dst_data[0],dst_outputsize,0);
			if(ret<0)
			{
				printf("Fill audio frame failed!\n");
			}
		
			this->streamServer->write_audio_frame(frame);
			av_freep(&src_data[0]);
			av_freep(&dst_data[0]);
			av_freep(&src_data);
			av_freep(&dst_data);
		}
		//======================
		 hr = audioCaptureClient->ReleaseBuffer(frameCount);
		if (FAILED(hr))	
		{
		  printf("IAudioCaptureClient::ReleaseBuffer failed on pass %u: hr = 0x%08x\n", debugFrameCounter, hr);
		  audioClient->Stop();
		  audioCaptureClient->Release();
		  audioClient->Release();  
		  runFlag=false;
		  return;            
		}
	}
	audioClient->Stop();
	audioCaptureClient->Release();
	audioClient->Release();
}
void ISoundCapturer::stopFrameLoop()
{
	runFlag=false;
}
void ISoundCapturer::setStreamServer(StreamServer * streamServer)
{
	this->streamServer=streamServer;
}
bool ISoundCapturer::initISoundCapturer()
{
	frame=avcodec_alloc_frame();
	if(frame==NULL)
	{
		printf("AUDIO FRAME ALLOCATE\n");
		return false;
	}
	CoInitialize(0);
	CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), 
    (void**) &enumerator);
	enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
	HRESULT hr;
	hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**) &audioClient); 
	if (FAILED(hr)) {
		printf("IMMDevice::Activate(IAudioClient) failed: hr = 0x%08x", hr);
		return false;
	}
	
	hr = audioClient->GetMixFormat(&waveFormat);
	if (FAILED(hr)) 
	{
		printf("IAudioClient::GetMixFormat failed: hr = 0x%08x\n", hr);
		CoTaskMemFree(waveFormat);
		audioClient->Release();
		return false;
	}
	switch (waveFormat->wFormatTag) 
	{
	  case WAVE_FORMAT_IEEE_FLOAT:
		printf("Using IEEE:Channel:%d,SamplePerSec:%d\n",waveFormat->nChannels,waveFormat->nSamplesPerSec);
		waveFormat->wFormatTag = WAVE_FORMAT_PCM;
		waveFormat->wBitsPerSample = 16;
		waveFormat->nBlockAlign = waveFormat->nChannels * waveFormat->wBitsPerSample / 8;
		waveFormat->nAvgBytesPerSec = waveFormat->nBlockAlign * waveFormat->nSamplesPerSec;
		break;
		case WAVE_FORMAT_EXTENSIBLE:
		{
		  // naked scope for case-local variable
		  PWAVEFORMATEXTENSIBLE waveFormatEx = reinterpret_cast<PWAVEFORMATEXTENSIBLE>(waveFormat);
		  if (IsEqualGUID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, waveFormatEx->SubFormat)) 
		  {
			
			waveFormatEx->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
			waveFormatEx->Samples.wValidBitsPerSample = 16;
			waveFormat->wBitsPerSample = 16;
			waveFormat->nBlockAlign = waveFormat->nChannels * waveFormat->wBitsPerSample / 8;
			waveFormat->nAvgBytesPerSec = waveFormat->nBlockAlign * waveFormat->nSamplesPerSec;
			printf("Using IEEE_FLOAT_EXTENSIVE:Channel:%d,SamplePerSec:%d,BlockAlign:%d\n",waveFormat->nChannels,waveFormat->nSamplesPerSec,waveFormat->nBlockAlign);
		  } 
		  else 
		  {
			printf("Don't know how to coerce mix format to int-16\n");
			CoTaskMemFree(waveFormat);
			audioClient->Release();
			return false;
		  }
		}
		break;
	  default:
		printf("Don't know how to coerce WAVEFORMATEX with wFormatTag = 0x%08x to int-16\n", waveFormat->wFormatTag);
		CoTaskMemFree(waveFormat);
		audioClient->Release();
		return false;
	}
	
	
	  // call IAudioClient::Initialize
	  // note that AUDCLNT_STREAMFLAGS_LOOPBACK and AUDCLNT_STREAMFLAGS_EVENTCALLBACK do not work together...
	  // the "data ready" event never gets set, so we're going to do a timer-driven loop
	  hr = audioClient->Initialize(
		AUDCLNT_SHAREMODE_SHARED,
		AUDCLNT_STREAMFLAGS_LOOPBACK,
		10000000, 0, waveFormat, 0
		);
	   
	  if (FAILED(hr)) {
		printf("IAudioClient::Initialize failed: hr = 0x%08x\n", hr);
		audioClient->Release();
		return false;
	  }
	  //CoTaskMemFree(waveFormat);
	  //printf("This is for debug test:%d\n",waveFormat->nBlockAlign);
	  // activate an IAudioCaptureClient
	 
	  hr = audioClient->GetService(__uuidof(IAudioCaptureClient), (void**) &audioCaptureClient);
	  if (FAILED(hr)) {
		printf("IAudioClient::GetService(IAudioCaptureClient) failed: hr 0x%08x\n", hr);
		audioClient->Release();
		return false;
	  }

	  hr = audioClient->Start();
	  if (FAILED(hr)) {
		printf("IAudioClient::Start failed: hr = 0x%08x\n", hr);
		audioCaptureClient->Release();
		audioClient->Release();
		return false;
	  }
	  if(!this->setupSwscale())
	  {
		  printf("Failed setup scale\n");
		  return false;
	  }
	  runFlag=true;
	return true;


}

int ISoundCapturer::alloc_samples_array_and_data(uint8_t ***data, int *linesize, int nb_channels,
                                    int nb_samples, enum AVSampleFormat sample_fmt, int align)
{
    int nb_planes = av_sample_fmt_is_planar(sample_fmt) ? nb_channels : 1;
    *data = (uint8_t **)av_malloc(sizeof(*data) * nb_planes);
    if (!*data)
        return AVERROR(ENOMEM);
    return av_samples_alloc(*data, linesize, nb_channels,
                            nb_samples, sample_fmt, align);
}