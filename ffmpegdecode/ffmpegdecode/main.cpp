#include "StreamClient.h"
#include "Player.h"
int main()
{
	setLocalPort(1234);
	setupStreamClient();
	initDecoder();
	pair<char*,int> data;
	while(true)
	{
		if(recvData(data))
		{
			decode_frame(data);
		}
		else
		{
			Sleep(10);
		}
	}
}