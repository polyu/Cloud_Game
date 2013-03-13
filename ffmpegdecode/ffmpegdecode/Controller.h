#include <SDL.h>
#include <Windows.h>
struct KeyEvent
{
	int keyCode1;
	int keyCode2;
};
struct MouseEvent
{
	int clickedButton;
	int relx;
	int rely;
};
class Controller
{
public:
	Controller();
	~Controller();
	bool initControllerClient();
	bool sendKeyEvent(int key1,int key2);
	bool sendMouseEvent(int relx,int rely,int clickButton);
private:
	int sdlkeyCodeTransfer(int sdlKeyCode);
	bool sendOutEvent(char *event,int size);
};

