#include <windows.h>
#include "ISoundCapturer.h"
#include "IVideoCapturer.h"
#include "StreamServer.h"
int main(int argc, char* argv[])
{
	StreamServer server;
	IVideoCapturer vcapturer;
	server.setRemoteAddress("127.0.0.1",1234);
	if(!server.startStreamServer())
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