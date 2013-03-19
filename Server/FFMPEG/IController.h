#include "stdafx.h"
#include <windows.h>
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
class IController
{
public:
	IController();
	~IController();
	bool initIController();
	void setLocalPort(int port);
	void startControllerLoop();
	void stopControllerLoop();
private:
	bool sendKeyboardEvent(int virtualKeyCode1,int virtualKeyCode2);
	bool sendMouseEvent(int relx,int rely,int button,int updown);
	SOCKET fd;
	int localPort;
	bool runFlag;
	SOCKADDR_IN localAddr;
	
};