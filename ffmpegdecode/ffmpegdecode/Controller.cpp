#include "Controller.h"
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
		case SDLK_CAPSLOCK:
			return VK_CAPITAL;
			break;
		case SDLK_RIGHT:
			return VK_RIGHT;
			break;
		case SDLK_LEFT:
			return VK_RIGHT;
			break;
		case SDLK_DOWN:
			return VK_DOWN;
			break;
		case SDLK_UP:
			return VK_UP;
			break;
		case SDLK_BACKSPACE:
			return VK_BACK;
			break;
		case SDLK_SPACE:
			return VK_SPACE;
			break;
		case SDLK_RETURN:
			return VK_RETURN;
			break;
		case SDLK_LCTRL:
		case SDLK_RCTRL:
		case KMOD_LCTRL:
		case KMOD_RCTRL:
			return VK_CONTROL;
			break;
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
		case KMOD_LSHIFT:
		case KMOD_RSHIFT:
			return VK_SHIFT;
			break;
		case SDLK_LALT:
		case SDLK_RALT:
		case KMOD_LALT:
		case KMOD_RALT:
			return VK_MENU;
			break;
		case SDLK_DELETE:
			return VK_DELETE;
			break;
		case SDLK_TAB:
			return VK_TAB;
			break;
		case SDLK_COLON:
		case SDLK_SEMICOLON:
			return VK_OEM_1;
			break;
		case SDLK_BACKQUOTE:
			return VK_OEM_3;
			break;
		case SDLK_COMMA:
			return VK_OEM_COMMA;
			break;
		case SDLK_PERIOD:
			return VK_OEM_PERIOD;
			break;
		case SDLK_SLASH:
			return VK_OEM_2;
			break;
		case SDLK_QUOTE:
			return VK_OEM_7;
			break;
		case SDLK_LEFTBRACKET:
			return VK_OEM_4;
			break;
		case SDLK_BACKSLASH:
			return VK_OEM_5;
			break;
		case SDLK_RIGHTBRACKET:
			return VK_OEM_6;
			break;
		case SDLK_MINUS:
			return VK_OEM_MINUS;
			break;
		case SDLK_EQUALS:
			return VK_OEM_PLUS;
			break;
			
		default:
			printf("Unsupport key\n");
			return -1;

	}
}