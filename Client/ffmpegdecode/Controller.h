#include "stdafx.h"
#include <SDL.h>
#include <Windows.h>
#include <udt.h>
#define KEYEVENT 1
#define MOUSEEVENT 2
#define PRESSDOWNDIRECTION 0
#define PRESSUPDIRECTION 1
struct ControlEvent
{
	int type;
	int keyCode1;
	int keyCode2;
	int relx;
	int rely;
	int clickedButton;
	int direction;
};
class Controller
{
public:
	Controller();
	~Controller();
	bool initControllerClient();
	bool sendKeyEvent(int key1,int key2);
	bool sendMouseEvent(int relx,int rely,int clickButton,int direction);
private:
	int sdlkeyCodeTransfer(int sdlKeyCode);
	bool sendOutEvent(char *event,int size);
	UDTSOCKET serv ;
	UDTSOCKET client ;
	sockaddr_in client_addr;
};

