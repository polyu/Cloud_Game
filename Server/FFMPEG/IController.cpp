#include "IController.h"
IController::IController()
{
	this->localPort=DEFAULT_CONTROLPORT;
	fd=0;
	this->runFlag=false;
}
IController::~IController()
{
	if(fd!=0)
	{
		closesocket(fd);
	}
}
bool IController::initIController()
{
	int retryTime=0;
	fd=socket(AF_INET,SOCK_DGRAM,0);
	if(fd==INVALID_SOCKET)
	{
		printf("Error init controller socket\n");
		return false;
	}
	localAddr.sin_family=AF_INET;   
	localAddr.sin_port=htons(this->localPort);   
	localAddr.sin_addr.s_addr = INADDR_ANY;
	if (bind(fd,(sockaddr*)&localAddr,sizeof(localAddr)) == SOCKET_ERROR) 
	{   
		printf("Error when bind controller socket\n");
        return false;
    }
	runFlag=true;
	return true;
}
void IController::startControllerLoop()
{
	char dataBuf[10240];
	
	while(runFlag)
	{
		int getSize=recvfrom(fd, dataBuf, 10240,0,NULL,NULL);
		if (getSize<=0)
		{
			printf( "recv message Error:%s\n" ,WSAGetLastError());
			break;
		}
		if(getSize!=sizeof(ControlEvent))
		{
			printf( "Bad procotol\n" );
			break;
		}
		ControlEvent *cevent=(ControlEvent*)dataBuf;
		if(cevent->type==KEYEVENT)
		{
			printf("Got Keyevent %d<-->%d\n",cevent->keyCode1,cevent->keyCode2);
			this->sendKeyboardEvent(cevent->keyCode1,cevent->keyCode2);
		}
		else if(cevent->type==MOUSEEVENT)
		{
			printf("Got Mouseevent%d<-->%d<-->%d<-->%d\n",cevent->relx,cevent->rely,cevent->clickedButton,cevent->direction);
			this->sendMouseEvent(cevent->relx,cevent->rely,cevent->clickedButton,cevent->direction);
		}
	}
}
void IController::stopControllerLoop()
{
	this->runFlag=false;
}


void IController::setLocalPort(int port)
{
	this->localPort=port;
}
bool IController::sendKeyboardEvent(int virtualKeyCode1,int virtualKeyCode2)
{
	printf("Push %d--->%d\n",virtualKeyCode1,virtualKeyCode2);
	if(virtualKeyCode2!=0)
	{
		keybd_event(virtualKeyCode2,0,0,0);
		 keybd_event(virtualKeyCode1,0,0,0);
		 keybd_event(virtualKeyCode1,0,KEYEVENTF_KEYUP,0);
		 keybd_event(virtualKeyCode2,0,KEYEVENTF_KEYUP,0);
	}
	else
	{
		keybd_event(virtualKeyCode1,0,0,0);
		keybd_event(virtualKeyCode1,0,KEYEVENTF_KEYUP,0);
	}
	return true;

}
bool IController::sendMouseEvent(int relx,int rely,int button,int direction)
{
	if(button==0)
	{
		mouse_event(MOUSEEVENTF_MOVE,relx,rely,0,0);
	}
	else
	{
		switch(button)
		{
			case 1:
				if(direction==PRESSDOWNDIRECTION )
				{
					mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);
				}
				else
				{
					mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);
				}
			break;
			case 3:
				if(direction==PRESSDOWNDIRECTION )
				{
					mouse_event(MOUSEEVENTF_RIGHTDOWN,0,0,0,0);
				}
				else
				{
					mouse_event(MOUSEEVENTF_RIGHTUP,0,0,0,0);
				}
			break;
			case 2:
				if(direction==PRESSDOWNDIRECTION )
				{
					mouse_event(MOUSEEVENTF_MIDDLEDOWN,0,0,0,0);
				}
				else
				{
					mouse_event(MOUSEEVENTF_MIDDLEUP,0,0,0,0);
				}
			break;
			case 4:
				mouse_event(MOUSEEVENTF_WHEEL,0,0,WHEEL_DELTA,0);
				break;
			case 5:
				mouse_event(MOUSEEVENTF_WHEEL,0,0,-WHEEL_DELTA,0);
				break;
			default:
				printf("Unknown Event!\n");
				return false;
		}
	}
	return true;
}

