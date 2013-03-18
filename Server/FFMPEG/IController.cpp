#include "IController.h"
IController::IController()
{
	this->remoteAddr=DEFAULT_REMOTEADDRESS;
	this->remotePort=DEFAULT_REMOTECONTROLPORT;
	socket=0;
	this->runFlag=false;
}
IController::~IController()
{
	if(socket!=0)
	{
		UDT::close(socket);
	}
}
bool IController::initIController()
{
	int retryTime=0;
	
	
	if(!this->establishConntection())
	{
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
		int getSize=UDT::recvmsg(socket, dataBuf, 10240);
		if (UDT::ERROR == getSize)
		{
			printf( "recv message Error:%s\n" ,UDT::getlasterror().getErrorMessage());
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

bool IController::establishConntection()
{

	socket = UDT::socket(AF_INET, SOCK_DGRAM, 0);
	sockaddr_in remote_addr;
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_port = htons(this->remotePort);
	remote_addr.sin_addr.s_addr=inet_addr(remoteAddr.c_str());
	memset(&(remote_addr.sin_zero), '\0', 8);
	printf("Try connecting remote client\n");
	int retryTime=0;
	bool connected=false;
	while(retryTime<3)
	{
		retryTime++;
		if (UDT::ERROR == UDT::connect(socket, (sockaddr*)&remote_addr, sizeof(remote_addr)))
		{
			printf("connect error: %s\n", UDT::getlasterror().getErrorMessage());
			continue;
		}
		connected=true;
		break;
	}
	if(!connected)
	{
		printf("Connected Remote Failed\n");
		return false;
	}
	printf("Client connected\n");
	return true;
}
void IController::setRemoteAddress(string address,int remotePort)
{
	this->remoteAddr=address;
	this->remotePort=remotePort;
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

