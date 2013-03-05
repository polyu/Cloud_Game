#include <windows.h>
#include "VideoEncoder.h"
#include "StreamServer.h"
int main(int argc, char* argv[])
{
	StreamServer server;
	server.setDestAddress("127.0.0.1",1234);
	server.startServer();
	VideoEncoder encoder;
	encoder.setStreamServer(&server);
	encoder.initEncoder();
	encoder.encodeFrameLoop();
	//encoder.debugEncoder("c:/test.264");
	return 0;
}