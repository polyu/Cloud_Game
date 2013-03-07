// SoundStreamer.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include <windows.h>
#include <audioclient.h>
#include <stdio.h>
#include <avrt.h>
#include <iostream>
#include <fstream>
#include <vector> 
#include <iterator>
#include <mmsystem.h>
#include <mmdeviceapi.h>



int _tmain(int argc, _TCHAR* argv[]) {
  IMMDeviceEnumerator *enumerator = 0;
  IMMDevice *device = 0;

  CoInitialize(0);
  CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), 
    (void**) &enumerator);
  enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);

  HANDLE processOutWrite, processOutRead, processInWrite, processInRead;    

  /*wchar_t processCommand[2000];
  {
    FILE* commandFile;
    fopen_s(&commandFile, "command.txt", "r");
    char cmd[2000];
    fread(cmd, sizeof(char), 2000, commandFile);
    fclose(commandFile);
    size_t count;
    mbstowcs_s(&count, processCommand, cmd, 2000);
  }*/

  /*{	
    //create pipes for plink process
    SECURITY_ATTRIBUTES pipeAttributes = {0};
    pipeAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    pipeAttributes.bInheritHandle = TRUE;
    pipeAttributes.lpSecurityDescriptor= NULL;
    CreatePipe(&processOutRead, &processOutWrite, &pipeAttributes, 0);
    CreatePipe(&processInRead,  &processInWrite,  &pipeAttributes, 0);
    STARTUPINFO startupInfo;
    ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
    startupInfo.cb = sizeof(STARTUPINFO);
    startupInfo.hStdError = processOutWrite;
    startupInfo.hStdOutput = processOutWrite;
    startupInfo.hStdInput = processInRead;
    startupInfo.dwFlags |= STARTF_USESTDHANDLES;
    PROCESS_INFORMATION processInfo = {0};
    //launch process
    CreateProcess(NULL, processCommand, NULL, NULL, TRUE, 0, NULL, NULL, &startupInfo, &processInfo);

    //wait for plink to connect to minimze sound delay  (magic number)
    Sleep(2500);

  }*/

  HRESULT hr;
  // activate an IAudioClient
  IAudioClient *audioClient;
  hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**) &audioClient); 
  if (FAILED(hr)) {
    printf("IMMDevice::Activate(IAudioClient) failed: hr = 0x%08x", hr);
    return hr;
  }

  // get the default device format
  WAVEFORMATEX *waveFormat;
  hr = audioClient->GetMixFormat(&waveFormat);
  if (FAILED(hr)) {
    printf("IAudioClient::GetMixFormat failed: hr = 0x%08x\n", hr);
    CoTaskMemFree(waveFormat);
    audioClient->Release();
    return hr;
  }

  // coerce int-16 wave format
  // can do this in-place since we're not changing the size of the format
  // also, the engine will auto-convert from float to int for us
  switch (waveFormat->wFormatTag) {
  case WAVE_FORMAT_IEEE_FLOAT:
    waveFormat->wFormatTag = WAVE_FORMAT_PCM;
    waveFormat->wBitsPerSample = 16;
    waveFormat->nBlockAlign = waveFormat->nChannels * waveFormat->wBitsPerSample / 8;
    waveFormat->nAvgBytesPerSec = waveFormat->nBlockAlign * waveFormat->nSamplesPerSec;
    break;

  case WAVE_FORMAT_EXTENSIBLE:
    {
      // naked scope for case-local variable
      PWAVEFORMATEXTENSIBLE waveFormatEx = reinterpret_cast<PWAVEFORMATEXTENSIBLE>(waveFormat);
      if (IsEqualGUID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, waveFormatEx->SubFormat)) {
        waveFormatEx->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
        waveFormatEx->Samples.wValidBitsPerSample = 16;
        waveFormat->wBitsPerSample = 16;
        waveFormat->nBlockAlign = waveFormat->nChannels * waveFormat->wBitsPerSample / 8;
        waveFormat->nAvgBytesPerSec = waveFormat->nBlockAlign * waveFormat->nSamplesPerSec;
      } else {
        printf("Don't know how to coerce mix format to int-16\n");
        CoTaskMemFree(waveFormat);
        audioClient->Release();
        return E_UNEXPECTED;
      }
    }
    break;

  default:
    printf("Don't know how to coerce WAVEFORMATEX with wFormatTag = 0x%08x to int-16\n", waveFormat->wFormatTag);
    CoTaskMemFree(waveFormat);
    audioClient->Release();
    return E_UNEXPECTED;
  }

  UINT32 blockAlign = waveFormat->nBlockAlign;

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
    return hr;
  }
  CoTaskMemFree(waveFormat);

  // activate an IAudioCaptureClient
  IAudioCaptureClient *audioCaptureClient;
  hr = audioClient->GetService(__uuidof(IAudioCaptureClient), (void**) &audioCaptureClient);
  if (FAILED(hr)) {
    printf("IAudioClient::GetService(IAudioCaptureClient) failed: hr 0x%08x\n", hr);
    audioClient->Release();
    return hr;
  }

  hr = audioClient->Start();
  if (FAILED(hr)) {
    printf("IAudioClient::Start failed: hr = 0x%08x\n", hr);
    audioCaptureClient->Release();
    audioClient->Release();
    return hr;
  }

  // loopback capture loop
  for (UINT32 i = 0; true; i++) {
    UINT32 nextPacketSize;
    hr = audioCaptureClient->GetNextPacketSize(&nextPacketSize);
    if (FAILED(hr)) {
      printf("IAudioCaptureClient::GetNextPacketSize failed on pass %u: hr = 0x%08x\n", i, hr);
      audioClient->Stop();
      audioCaptureClient->Release();
      audioClient->Release();            
      return hr;
    }

    if (nextPacketSize == 0) { // no data yet
      continue;
    }

    // get the captured data
    BYTE *data;
    UINT32 frameCount;
    DWORD bufferFlags;

    hr = audioCaptureClient->GetBuffer(&data, &frameCount, &bufferFlags, NULL, NULL);
    if (FAILED(hr)) {
      printf("IAudioCaptureClient::GetBuffer failed on pass %u: hr = 0x%08x\n", i, hr);
      audioClient->Stop();
      audioCaptureClient->Release();
      audioClient->Release();            
      return hr;            
    }

    if (bufferFlags & AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY) {
      printf("IAudioCaptureClient::GetBuffer reports 'data discontinuity' on pass %u\n", i);
    }
    if (bufferFlags & AUDCLNT_BUFFERFLAGS_SILENT) {
      printf("IAudioCaptureClient::GetBuffer reports 'silent' on pass %u\n", i);
    }
    if (bufferFlags & AUDCLNT_BUFFERFLAGS_TIMESTAMP_ERROR) {
      printf("IAudioCaptureClient::GetBuffer reports 'timestamp error' on pass %u\n", i);
    }

    if (frameCount == 0) {
      printf("IAudioCaptureClient::GetBuffer said to read 0 frames on pass %u\n", i);
      audioClient->Stop();
      audioCaptureClient->Release();
      audioClient->Release();            
      return E_UNEXPECTED;            
    }

    LONG bytesToWrite = frameCount * blockAlign;
    DWORD bytesWritten;
	printf("Recording");
    /*WriteFile(processInWrite,reinterpret_cast<PCHAR>(data), bytesToWrite, &bytesWritten, NULL);
    if (bytesWritten != bytesToWrite) {
      printf("WriteFile: tried to write %d bytes, but %d bytes written\n", bytesToWrite, bytesWritten);            
    }

    char buf[10000];
    DWORD count;
    DWORD bytesAvailable;
    PeekNamedPipe(processOutRead, NULL, 0, 0, &bytesAvailable, NULL);
    if (bytesAvailable > 0) { 
      ReadFile(processOutRead, buf, 10000, &count, NULL);
      std::cout.write(buf, count);
    }*/


    hr = audioCaptureClient->ReleaseBuffer(frameCount);
    if (FAILED(hr)) {
      printf("IAudioCaptureClient::ReleaseBuffer failed on pass %u: hr = 0x%08x\n", i, hr);
      audioClient->Stop();
      audioCaptureClient->Release();
      audioClient->Release();            
      return hr;            
    }

  } // capture loop

  audioClient->Stop();
  audioCaptureClient->Release();
  audioClient->Release();

  return 0;
}

