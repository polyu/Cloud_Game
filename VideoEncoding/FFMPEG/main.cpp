#include "StreamServer.h"
#include "ISoundCapturer.h"
#include "IVideoCapturer.h"


int main(int argc, char* argv[])
{
	StreamServer server;
	IVideoCapturer vcapturer;
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
	vcapturer.startFrameLoop();

}