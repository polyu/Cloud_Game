#include "StreamServer.h"
#include "ISoundCapturer.h"
#include "IVideoCapturer.h"
#include <process.h>
StreamServer server;
IVideoCapturer vcapturer;
ISoundCapturer acapturer;
void videoCaptureThread(void*)
{
	vcapturer.startFrameLoop();
}
void audioCaptureThread(void*)
{
	acapturer.startFrameLoop();
}
int main(int argc, char* argv[])
{
	
	if(!server.initStreamServer())
	{
		printf("Stream Server Failed\n");
		system("pause");
		return -1;	
	}
	printf("Stream Server Start\n");
	if(!vcapturer.initVideoCapture())
	{
		printf("Video Capture Server Failed\n");
		return -1;	
	}
	vcapturer.setStreamServer(&server);
	if(!acapturer.initISoundCapturer())
	{
		printf("Audio Capture Server Failed\n");
		return -1;	
	}
	acapturer.setStreamServer(&server);
	_beginthread(videoCaptureThread,0,NULL);
	_beginthread(audioCaptureThread,0,NULL);
	while(true)
	{
		Sleep(1000);
	}
}