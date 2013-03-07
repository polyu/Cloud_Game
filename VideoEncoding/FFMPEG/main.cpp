#include <windows.h>
#include "VideoEncoder.h"
#include "StreamServer.h"
#include "SoundEncoder.h"
int main(int argc, char* argv[])
{
	/*StreamServer server;
	server.setDestAddress("127.0.0.1",1234);
	server.startServer();
	VideoEncoder encoder;
	encoder.setStreamServer(&server);
	encoder.initEncoder();
	encoder.encodeFrameLoop();
	//encoder.debugEncoder("c:/test.264");*/
	SoundEncoder encoder;
	if(encoder.initSoundCapture())
	{
		printf("Init OK");
	}
	else
		return 1;
	return 0;
}