#include "Controller.h"
Controller::Controller()
{
	serv=0;
	client=0;
}
Controller::~Controller()
{
	UDT::close(serv);
	UDT::close(client);
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
	int sendSize=UDT::sendmsg(client,data,size,-1,true);
	return sendSize==size;
	return true;
}
bool Controller::initControllerClient()
{
	serv = UDT::socket(AF_INET, SOCK_DGRAM, 0);
	sockaddr_in my_addr;
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(DEFAULT_CONTROLLERPORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(my_addr.sin_zero), '\0', 8);
	if (UDT::ERROR == UDT::bind(serv, (sockaddr*)&my_addr, sizeof(my_addr)))
	{
		printf("bind: %s\n" , UDT::getlasterror().getErrorMessage());
		return false;
	}
	UDT::listen(serv,1);
	int namelen;
	printf("Waiting remote server's connection\n");
	client = UDT::accept(serv, (sockaddr*)&client_addr, &namelen);
	//printf( "new connection: %s:%s\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port) );
	printf("Connectiong established\n");
	bool blockingFlag=false;
	int ret=UDT::setsockopt(client,0,UDT_SNDSYN,(const char*)(&blockingFlag),sizeof(bool));
	if(ret!=0)
	{
		printf("Set unblocking send failed\n");
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