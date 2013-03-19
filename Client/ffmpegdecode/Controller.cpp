#include "Controller.h"
Controller::Controller()
{
	fd=0;
	this->setRemoteAddress(LOCALADDRESS,DEFAULT_CONTROLLERPORT);
}
Controller::~Controller()
{
	if(fd!=0)
	{
		closesocket(fd);
	}
}
void Controller::setRemoteAddress(string remoteAddr,int port)
{
	this->remoteAddr.sin_port=htons(port);
	this->remoteAddr.sin_family=AF_INET;
	this->remoteAddr.sin_addr.s_addr=inet_addr(remoteAddr.c_str());
}
bool Controller::sendKeyEvent(int key1,int key2)
{
	ControlEvent kevent;
	memset(&kevent,0,sizeof(kevent));
	kevent.type=KEYEVENT;
	int temp=sdlkeyCodeTransfer(key1);
	if(temp==-1)
	{
		printf("Ignorg this key event\n");
		return false;
	}
	kevent.keyCode1=temp;
	if(key2!=0)
	{
		temp=sdlkeyCodeTransfer(key2);
		if(temp!=-1)
			kevent.keyCode2=temp;
	}
	return sendOutEvent((char *)(&kevent),sizeof(kevent));
}
bool Controller::sendMouseEvent(int relx,int rely,int clickButton,int direction)
{
	ControlEvent mevent;
	memset(&mevent,0,sizeof(mevent));
	mevent.type=MOUSEEVENT;
	if(clickButton==0)
	{
		mevent.relx=relx;
		mevent.rely=rely;
	}
	else
	{
		mevent.clickedButton=clickButton;
		mevent.direction=direction;
	}
	return sendOutEvent((char *)(&mevent),sizeof(mevent));
}
bool Controller::sendOutEvent(char *data,int size)
{
	int sendSize=sendto(fd,data,size,0,(const sockaddr *)&this->remoteAddr,sizeof(this->remoteAddr));
	return sendSize==size;
	
}
bool Controller::initControllerClient()
{
	fd=socket(AF_INET, SOCK_DGRAM, 0);
	if(fd==INVALID_SOCKET)
	{
		printf("failed to init controller socket\n");
		return false;
	}
	return true;
}
int Controller::sdlkeyCodeTransfer(int sdlKeyCode)
{
	
	if(sdlKeyCode>=SDLK_a&&sdlKeyCode<=SDLK_z)// captial
	{
		return 65+sdlKeyCode-SDLK_a;
	}
	if(sdlKeyCode>=SDLK_0&&sdlKeyCode<=SDLK_9)
	{
		return 48+sdlKeyCode-SDLK_0;
	}
	switch(sdlKeyCode)
	{
		case SDLK_ESCAPE:
			return VK_ESCAPE;
			
		case SDLK_CAPSLOCK:
			return VK_CAPITAL;
			
		case SDLK_RIGHT:
			return VK_RIGHT;
			
		case SDLK_LEFT:
			return VK_RIGHT;
			
		case SDLK_DOWN:
			return VK_DOWN;
			
		case SDLK_UP:
			return VK_UP;
		
		case SDLK_BACKSPACE:
			return VK_BACK;
			
		case SDLK_SPACE:
			return VK_SPACE;
			
		case SDLK_RETURN:
			return VK_RETURN;
			
		case SDLK_LCTRL:
		case SDLK_RCTRL:
		case KMOD_LCTRL:
		case KMOD_RCTRL:
			return VK_CONTROL;
			
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
		case KMOD_LSHIFT:
		case KMOD_RSHIFT:
			return VK_SHIFT;
			
		case SDLK_LALT:
		case SDLK_RALT:
		case KMOD_LALT:
		case KMOD_RALT:
			return VK_MENU;
			
		case SDLK_DELETE:
			return VK_DELETE;
			
		case SDLK_TAB:
			return VK_TAB;
			
		case SDLK_COLON:
		case SDLK_SEMICOLON:
			return VK_OEM_1;
			
		case SDLK_BACKQUOTE:
			return VK_OEM_3;
			
		case SDLK_COMMA:
			return VK_OEM_COMMA;
			
		case SDLK_PERIOD:
			return VK_OEM_PERIOD;
			
		case SDLK_SLASH:
			return VK_OEM_2;
			
		case SDLK_QUOTE:
			return VK_OEM_7;
			
		case SDLK_LEFTBRACKET:
			return VK_OEM_4;
			
		case SDLK_BACKSLASH:
			return VK_OEM_5;
			
		case SDLK_RIGHTBRACKET:
			return VK_OEM_6;
			
		case SDLK_MINUS:
			return VK_OEM_MINUS;
			
		case SDLK_EQUALS:
			return VK_OEM_PLUS;
			
			
		default:
			printf("Unsupport key\n");
			return -1;

	}
}