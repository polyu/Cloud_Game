#include "stdafx.h"
#include <SDL.h>
#include <Windows.h>
#include <string>
using namespace std;
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
	void setRemoteAddress(string remoteAddr,int port);
private:
	int sdlkeyCodeTransfer(int sdlKeyCode);
	bool sendOutEvent(char *event,int size);
	SOCKET fd;
	sockaddr_in remoteAddr;
	
};

