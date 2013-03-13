#include "IController.h"
IController::IController()
{
}
IController::~IController()
{

}
bool IController::sendKeyboardEvent(int virtualKeyCode1,int virtualKeyCode2,bool together)
{
	printf("Push %d--->%d>>>>Together:%d\n",virtualKeyCode1,virtualKeyCode2,together);
	if(together)
	{
		 keybd_event(virtualKeyCode1,0,KEYEVENTF_KEYUP,0);
		 keybd_event(virtualKeyCode2,0,KEYEVENTF_KEYUP,0);
	}
	else
	{
		keybd_event(virtualKeyCode1,0,0,0);
	}
	return true;

}

