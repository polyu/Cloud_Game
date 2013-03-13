#include "stdafx.h"
#include <Windows.h>
class IController
{
public:
	IController();
	~IController();
	bool initIController();
	bool establishConntection();
	bool establishProxyConnection();
	
private:
	bool sendKeyboardEvent(int virtualKeyCode1,int virtualKeyCode2,bool together);
	bool sendMouseEvent();
	
};