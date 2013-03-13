//------------------------------------------------------------------------------------------
//
//	DXhook.cpp - A dll used to intercept function calls to dx9. It needs to be injected to
//				 a compiled binary file (.exe) in order to hook it correctly.
//											Implemented by Omer Ofir and Roey Ben-Haim
//				
//------------------------------------------------------------------------------------------
//
//	DXhook library was written with the help of Wiccaan's post in www.extalia.com:
//	http://www.extalia.com/forums/viewtopic.php?f=45&t=2578
//
//------------------------------------------------------------------------------------------

#pragma comment(lib, "Detours\\detours.lib")
#include "stdafx.h"
#include "Detours\detours.h"
#include "ID3D9Wrapper.h"
#include <iostream>
#include <fstream>
#include <string>
#include <xstring>
using std::ofstream;
using std::ios;
using std::string;
using std::wstring;

//	-------------------------------	Macros ----------------------------------------
#define LOGFILE(text)		if (false)\
							{ofstream log;\
							log.open("D:\\log.txt", ios::app);\
							log << (text);\
							log.close();}

//	-------------------------------	Forward Decelerations -------------------------
LPSTR UnicodeToAnsi(LPCWSTR s);
void StyleSettings(DWORD &dwExStyle, DWORD &dwStyle);
LRESULT CALLBACK Mine_WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

//	-------------------------------	Global Variables ------------------------------
HANDLE	HookHandle		= NULL;
HWND	GameHwnd		= NULL;
DWORD	dwGameStyle		= NULL;
DWORD	dwGameStyleEx	= NULL;

//------------------------------------------------------------------------------------------
//	Defining the trampoline prototype for the detour to intercept.
//	It's the assential, Direct3DCreate9, which is the basis for every DX based game.
//------------------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif

	//--------------------------------------------------------------------------------------
	//	DirectX detoured function
	//--------------------------------------------------------------------------------------
	DETOUR_TRAMPOLINE(IDirect3D9 *WINAPI Real_Direct3DCreate9(UINT SDKVersion), Direct3DCreate9);

	//--------------------------------------------------------------------------------------
	//	Various windows API detoured functions
	//--------------------------------------------------------------------------------------
	DETOUR_TRAMPOLINE(	HWND	WINAPI Real_CreateWindowExA		(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam), CreateWindowExA);
	DETOUR_TRAMPOLINE(	HWND	WINAPI Real_CreateWindowExW		(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam), CreateWindowExW);
	DETOUR_TRAMPOLINE(	ATOM	WINAPI Real_RegisterClassExW	(CONST WNDCLASSEXW* wndClass), RegisterClassExW);
	DETOUR_TRAMPOLINE(	ATOM	WINAPI Real_RegisterClassExA	(CONST WNDCLASSEXA* wndClass), RegisterClassExA);

#ifdef __cplusplus
}
#endif

//	----------------------------------------------------------------------
//	----------------------------------------------------------------------
//
//							Our Intercepted Functions
//
//	----------------------------------------------------------------------
//	----------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//	Function name:	Mine_Direct3DCreate9
//	Description:	The trampoline for Direct3DCreate9 call. In it we create our own IDirect3D9 
//					object, which inherits IDirect3D9. It enables us to manipulate calls used
//					for this device as we are the middle man in this configuration.
//	Parameters:		SDKVersion : The SDK version for us to use.
//	Returns:		Our IDirect3D9 instance which encapsulate a real IDirect3D9
//------------------------------------------------------------------------------------------
IDirect3D9* WINAPI Mine_Direct3DCreate9(UINT SDKVersion)
{
	LOGFILE("Direct3DCreate9 called.\n");
	IDirect3D9* Direct3D = Real_Direct3DCreate9(SDKVersion);
	IDirect3D9* Mine_Direct3D = new Direct3D9Wrapper(Direct3D);
	return Mine_Direct3D;
}

//------------------------------------------------------------------------------------------
//	Function name:	Mine_CreateWindowExA
//	Description:	The trampoline for CreateWindowExA call. We use this intercepted function
//					to try and figure out the application main window handler for later keyboard
//					input recieving. 
//	Parameters:		Regular CreateWindowExA Parameters. We don't really need to know what they do
//					as our only purpose is to send it to a CreateWindowExA call, initiated by us.
//	Returns:		The new window handler
//------------------------------------------------------------------------------------------
HWND WINAPI Mine_CreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	if (nWidth > 280 && nWidth < 100000 && !GameHwnd){	//	This might be our game window

		//	************************	Game window settings override		***********************
		StyleSettings(dwExStyle, dwStyle);

		string winName(lpClassName);

		char tmp[256];
		sprintf(tmp, "Entered CreateWindowExW. name: '%s'. size: (%d, %d)\n", winName.c_str() ,nWidth, nHeight);
		LOGFILE(tmp);

		GameHwnd = Real_CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		return GameHwnd;
	}

	return Real_CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

//------------------------------------------------------------------------------------------
//	Function name:	Mine_CreateWindowExW
//	Description:	The trampoline for CreateWindowExW call. Same as the function above, only this
//					function deals with unicode strings (as the window name etc).
//	Parameters:		Regular CreateWindowExW Parameters. We don't really need to know what they do
//					as our only purpose is to send it to a CreateWindowExA call, initiated by us.
//	Returns:		The new window handler
//------------------------------------------------------------------------------------------
HWND WINAPI Mine_CreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	if (nWidth > 280 && nWidth < 100000 && !GameHwnd){	//	This might be our game window

		//	************************	Game window settings override		***********************
		StyleSettings(dwExStyle, dwStyle);

		string winName(UnicodeToAnsi(lpWindowName));

		char tmp[256];
		sprintf(tmp, "Entered CreateWindowExW. name: '%s'. size: (%d, %d)\n", winName.c_str() ,nWidth, nHeight);
		LOGFILE(tmp);

		winName = UnicodeToAnsi(lpClassName);
		sprintf(tmp, "Classname: '%s'.\n", winName.c_str());
		LOGFILE(tmp);

		GameHwnd = Real_CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		return GameHwnd;
	}

	return Real_CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

//------------------------------------------------------------------------------------------
//	Function name:	Mine_RegisterClassExW
//	Description:	The trampoline for RegisterClassExW call. Used for debug purposes, in order 
//					to try and figure out how a certain application is executing.
//	Parameters:		WNDCLASSEXW - a class containing fields for class registration
//	Returns:		Operation is\not successful
//------------------------------------------------------------------------------------------
ATOM WINAPI Mine_RegisterClassExW(WNDCLASSEXW *wndClass)
{
	if (wndClass){
		char tmp[256];
		LOGFILE("Mine_RegisterClassExW: ");

		if (wndClass->lpszClassName){
			string wndClassName(UnicodeToAnsi(wndClass->lpszClassName));
			sprintf(tmp, "className: %s", wndClassName.c_str());
			LOGFILE(tmp);
		}

		if (wndClass->lpszMenuName){
			string wndMenuName(UnicodeToAnsi(wndClass->lpszMenuName));
			sprintf(tmp, "menuName: %s", wndMenuName.c_str());
			LOGFILE(tmp);
		}

		if (wndClass->lpfnWndProc){
			LOGFILE("There's a WNDPROC.");
		}

		LOGFILE("\n");
	}
	
	return Real_RegisterClassExW(wndClass);
}

//------------------------------------------------------------------------------------------
//	Function name:	Mine_RegisterClassExA
//	Description:	The trampoline for RegisterClassExA call. Same as above. Difference with
//					the strings.
//	Parameters:		WNDCLASSEXW - a class containing fields for class registration
//	Returns:		Operation is\not successful
//------------------------------------------------------------------------------------------
ATOM WINAPI Mine_RegisterClassExA (CONST WNDCLASSEXA* wndClass)
{
#ifdef LOG_REGISTERCLASS
	LOGFILE(Mine_RegisterClassExA, "Mine_RegisterClassExA");
#endif

	return Real_RegisterClassExA(wndClass);
}

//	----------------------------------------------------------------------
//	----------------------------------------------------------------------
//
//							The hooking operations
//
//	----------------------------------------------------------------------
//	----------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//	Function name:	HookAPI
//	Description:	Hooking the real functions and our intercepted ones.
//------------------------------------------------------------------------------------------
void HookAPI()
{
	LOGFILE("----------------------------- Hooking API ---------------------\n");	
	DetourFunctionWithTrampoline( (PBYTE)Real_Direct3DCreate9, (PBYTE)Mine_Direct3DCreate9);
	DetourFunctionWithTrampoline( (PBYTE)Real_CreateWindowExA,	(PBYTE)Mine_CreateWindowExA );
	DetourFunctionWithTrampoline( (PBYTE)Real_CreateWindowExW,	(PBYTE)Mine_CreateWindowExW );
}

//------------------------------------------------------------------------------------------
//	Function name:	UnhookAPI
//	Description:	Unhooking the real functions and our intercepted ones, upon exit.
//------------------------------------------------------------------------------------------
void UnhookAPI()
{
	LOGFILE("----------------------------- Unhooking API ---------------------\n");	
	DetourRemove( (PBYTE)Real_Direct3DCreate9, (PBYTE)Mine_Direct3DCreate9);
	DetourRemove( (PBYTE)Real_CreateWindowExA,	(PBYTE)Mine_CreateWindowExA );
	DetourRemove( (PBYTE)Real_CreateWindowExW,	(PBYTE)Mine_CreateWindowExW );
}

//	----------------------------------------------------------------------
//	----------------------------------------------------------------------
//
//							Helper functions
//
//	----------------------------------------------------------------------
//	----------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//	Function name:	UnicodeToAnsi
//	Description:	This function gets a string in LPCWSTR format and returns a LPSTR one.
//	Parameters:		The string in LPCWSTR format.
//	Returns:		The string in LPSTR format.
//------------------------------------------------------------------------------------------
LPSTR UnicodeToAnsi(LPCWSTR s)
{
	if (s==NULL) return NULL;
	int cw=lstrlenW(s);
	if (cw==0) {CHAR *psz=new CHAR[1];*psz='\0';return psz;}
	int cc=WideCharToMultiByte(CP_ACP,0,s,cw,NULL,0,NULL,NULL);
	if (cc==0) return NULL;
	CHAR *psz=new CHAR[cc+1];
	cc=WideCharToMultiByte(CP_ACP,0,s,cw,psz,cc,NULL,NULL);
	if (cc==0) {delete[] psz;return NULL;}
	psz[cc]='\0';
	return psz;
}

//------------------------------------------------------------------------------------------
//	Function name:	StyleSettings
//	Description:	Setting the style settings words for window creation.
//	Parameters:		dwExStyle	- Reference for the style DWORD need to be set.
//					dwStyle		- Reference for another style DWORD need to be set.
//------------------------------------------------------------------------------------------
void StyleSettings(DWORD &dwExStyle, DWORD &dwStyle)
{
	dwStyle = WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_OVERLAPPEDWINDOW;
	// ** Remove Extra Settings From The Style If They Exist
	if( dwExStyle & WS_EX_CLIENTEDGE )
		dwExStyle -= WS_EX_CLIENTEDGE;

	if( dwExStyle & WS_EX_DLGMODALFRAME )
		dwExStyle -= WS_EX_DLGMODALFRAME;

	if( dwExStyle & WS_EX_WINDOWEDGE )
		dwExStyle -= WS_EX_WINDOWEDGE;

	if( dwExStyle & WS_EX_STATICEDGE )
		dwExStyle -= WS_EX_STATICEDGE;
}

//------------------------------------------------------------------------------------------
//	Function name:	DllMain
//	Description:	The dll's main entry point
//	Parameters:		Parameters are used to determine the creation purpose.
//	Returns:		TRUE.
//------------------------------------------------------------------------------------------
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
		HookHandle = hModule;
		HookAPI();
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		UnhookAPI();
	}
	return TRUE;
}