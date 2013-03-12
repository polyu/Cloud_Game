#include "ISoundCapturer.h"
/*
HRESULT WriteWaveHeader(HMMIO hFile, LPCWAVEFORMATEX pwfx, MMCKINFO *pckRIFF, MMCKINFO *pckData) {
    MMRESULT result;

    // make a RIFF/WAVE chunk
    pckRIFF->ckid = MAKEFOURCC('R', 'I', 'F', 'F');
    pckRIFF->fccType = MAKEFOURCC('W', 'A', 'V', 'E');

    result = mmioCreateChunk(hFile, pckRIFF, MMIO_CREATERIFF);
    if (MMSYSERR_NOERROR != result) {
        printf("mmioCreateChunk(\"RIFF/WAVE\") failed: MMRESULT = 0x%08x\n", result);
        return E_FAIL;
    }
    
    // make a 'fmt ' chunk (within the RIFF/WAVE chunk)
    MMCKINFO chunk;
    chunk.ckid = MAKEFOURCC('f', 'm', 't', ' ');
    result = mmioCreateChunk(hFile, &chunk, 0);
    if (MMSYSERR_NOERROR != result) {
        printf("mmioCreateChunk(\"fmt \") failed: MMRESULT = 0x%08x\n", result);
        return E_FAIL;
    }

    // write the WAVEFORMATEX data to it
    LONG lBytesInWfx = sizeof(WAVEFORMATEX) + pwfx->cbSize;
    LONG lBytesWritten =
        mmioWrite(
            hFile,
            reinterpret_cast<PCHAR>(const_cast<LPWAVEFORMATEX>(pwfx)),
            lBytesInWfx
        );
    if (lBytesWritten != lBytesInWfx) {
        printf("mmioWrite(fmt data) wrote %u bytes; expected %u bytes\n", lBytesWritten, lBytesInWfx);
        return E_FAIL;
    }

    // ascend from the 'fmt ' chunk
    result = mmioAscend(hFile, &chunk, 0);
    if (MMSYSERR_NOERROR != result) {
        printf("mmioAscend(\"fmt \" failed: MMRESULT = 0x%08x\n", result);
        return E_FAIL;
    }
    
    // make a 'fact' chunk whose data is (DWORD)0
    chunk.ckid = MAKEFOURCC('f', 'a', 'c', 't');
    result = mmioCreateChunk(hFile, &chunk, 0);
    if (MMSYSERR_NOERROR != result) {
        printf("mmioCreateChunk(\"fmt \") failed: MMRESULT = 0x%08x\n", result);
        return E_FAIL;
    }

    // write (DWORD)0 to it
    // this is cleaned up later
    DWORD frames = 0;
    lBytesWritten = mmioWrite(hFile, reinterpret_cast<PCHAR>(&frames), sizeof(frames));
    if (lBytesWritten != sizeof(frames)) {
        printf("mmioWrite(fact data) wrote %u bytes; expected %u bytes\n", lBytesWritten, (UINT32)sizeof(frames));
        return E_FAIL;
    }

    // ascend from the 'fact' chunk
    result = mmioAscend(hFile, &chunk, 0);
    if (MMSYSERR_NOERROR != result) {
        printf("mmioAscend(\"fact\" failed: MMRESULT = 0x%08x\n", result);
        return E_FAIL;
    }

    // make a 'data' chunk and leave the data pointer there
    pckData->ckid = MAKEFOURCC('d', 'a', 't', 'a');
    result = mmioCreateChunk(hFile, pckData, 0);
    if (MMSYSERR_NOERROR != result) {
        printf("mmioCreateChunk(\"data\") failed: MMRESULT = 0x%08x\n", result);
        return E_FAIL;
    }

    return S_OK;
}

HRESULT FinishWaveFile(HMMIO hFile, MMCKINFO *pckRIFF, MMCKINFO *pckData) {
    MMRESULT result;

    result = mmioAscend(hFile, pckData, 0);
    if (MMSYSERR_NOERROR != result) {
        printf("mmioAscend(\"data\" failed: MMRESULT = 0x%08x\n", result);
        return E_FAIL;
    }

    result = mmioAscend(hFile, pckRIFF, 0);
    if (MMSYSERR_NOERROR != result) {
        printf("mmioAscend(\"RIFF/WAVE\" failed: MMRESULT = 0x%08x\n", result);
        return E_FAIL;
    }

    return S_OK;    
}Debug Function*/
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
	/*
	HMMIO hFile; 
	if ((hFile = mmioOpenA("C:/1.wav", NULL, 
    MMIO_WRITE)) != NULL) 
    printf("file open ok");
	else 
	  printf("file open failed");
    // File cannot be opened. 
	 //MMCKINFO ckRIFF = {0};
    //MMCKINFO ckData = {0};
    //WriteWaveHeader(hFile, waveFormat, &ckRIFF, &ckData);*/
	HRESULT hr;
	long debugFrameCounter=0;
	int frameBufferCursor=0;
	int frameBufferCounter=0;
	BYTE *frameBuffer=(BYTE *)malloc(SOUNDCAPTUREBUF);//2M
	//int c=0;
	while(runFlag)
	{
		//c++;
		//printf("%d\n",c);
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
			//printf("Why");
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
		  Sleep(1000);
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
		if( this->waveFormat->nChannels==2&&this->waveFormat->nSamplesPerSec==OUTPUTSAMPLERATE)//Not need to cumsom cpu
		{
			
				frame->nb_samples=frameCount;
				/*printf("FRAMECOUNT:%d\n",frame->nb_samples);
				FILE *f=fopen("c:/o1.dump","w");
				fwrite(data,bytesToWrite,1,f);
				fclose(f);*/
				ret=avcodec_fill_audio_frame(frame,2,AV_SAMPLE_FMT_S16,data,bytesToWrite,1);
				if(ret<0)
				{
					printf("Fill audio frame failed!\n");
				}
				//mmioWrite(hFile, reinterpret_cast<PCHAR>(frame->data[0]), bytesToWrite);
				/*FILE *f1=fopen("c:/o3.dump","w");
				printf("Writing O3 %d\n",frame->linesize[0]);
				fwrite(frame->data[0],bytesToWrite,1,f1);
				fclose(f1);*/
				//printf("Will write\n");
				this->streamServer->write_audio_frame(frame);
				
		}
		else
		{
			printf("Resample start\n");
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
	//FinishWaveFile(hFile, &ckData, &ckRIFF);
	free(frameBuffer);
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
