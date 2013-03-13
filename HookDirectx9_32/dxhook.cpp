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
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9tex.h>
#include <mhook.h>
#include "ID3D9Wrapper.h"
#include "Log.h"


//	-------------------------------	Forward Decelerations -------------------------



//	-------------------------------	Global Variables ------------------------------
HANDLE	HookHandle		= NULL;
HWND	GameHwnd		= NULL;


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
	IDirect3D9* (WINAPI *Real_Direct3DCreate9)(UINT SDKVersion)=Direct3DCreate9;

	//--------------------------------------------------------------------------------------
	//	Various windows API detoured functions
	//--------------------------------------------------------------------------------------
		HWND	(WINAPI *Real_CreateWindowExA)		(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)=CreateWindowExA;
		HWND	(WINAPI *Real_CreateWindowExW)		(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)= CreateWindowExW;
	

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
		//By Now We do Nothing Here
		GameHwnd = Real_CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		return GameHwnd;
	
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
	
		//We do nothing here
		GameHwnd = Real_CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		return GameHwnd;

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
	if (!Mhook_SetHook((PVOID*)&Real_Direct3DCreate9, Mine_Direct3DCreate9))
	{
		exit(-1);
	}
	
}

//------------------------------------------------------------------------------------------
//	Function name:	UnhookAPI
//	Description:	Unhooking the real functions and our intercepted ones, upon exit.
//------------------------------------------------------------------------------------------
void UnhookAPI()
{
	Mhook_Unhook((PVOID*)&Real_Direct3DCreate9);
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
		//DisableThreadLibraryCalls(hModule);
		HookHandle = hModule;
		HookAPI();
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		UnhookAPI();
	}
	return TRUE;
}