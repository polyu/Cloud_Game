// proxydll.cpp
#include "stdafx.h"
#include "Log.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <detours.h>
#include <detver.h>
#include "d3d8.h"
HANDLE	HookHandle		= NULL;

#ifdef __cplusplus
extern "C"
{
#endif
	//--------------------------------------------------------------------------------------
	//	DirectX detoured function
	//--------------------------------------------------------------------------------------
	IDirect3D8* (WINAPI *Real_Direct3DCreate8)(UINT SDKVersion)= Direct3DCreate8;

	//--------------------------------------------------------------------------------------
	//	Various windows API detoured functions
	//--------------------------------------------------------------------------------------
		HWND	(WINAPI *Real_CreateWindowExA)		(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)=CreateWindowExA;
		HWND	(WINAPI *Real_CreateWindowExW)		(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)= CreateWindowExW;
		
#ifdef __cplusplus
}
#endif


IDirect3D8* WINAPI Mine_Direct3DCreate8(UINT SDKVersion)
	{
	LOGFILE("Direct3DCreate8 called.\n");
	IDirect3D8* Direct3D = Real_Direct3DCreate8(SDKVersion);
	IDirect3D8* Mine_Direct3D = new myIDirect3D8(Direct3D);
	return Mine_Direct3D;
	}

//------------------------------------------------------------------------------------------
//	Function name:	HookAPI
//	Description:	Hooking the real functions and our intercepted ones.
//------------------------------------------------------------------------------------------
void HookAPI()
{
	if(DetourTransactionBegin() != NO_ERROR)
	{
		LOGFILE("Hook Transcation Failed");	
		exit(-1);
	}
	DetourUpdateThread(GetCurrentThread());
	if(DetourAttach(&(PVOID&)Real_Direct3DCreate8, Mine_Direct3DCreate8) != NO_ERROR)
	{
		LOGFILE("Hook Directx 3d Failed");	
		exit(-1);
	}
	/*if(DetourAttach(&(PVOID&)Real_CreateWindowExA,Mine_CreateWindowExA) != NO_ERROR)
	{
		LOGFILE("Hook CreateWindowsExA Failed");	
		exit(-1);
	}
	if(DetourAttach(&(PVOID&)Real_CreateWindowExW, Mine_CreateWindowExW) != NO_ERROR)
	{
		LOGFILE("Hook CreateWindowsExw Failed");	
		exit(-1);
	}*/

	if(DetourTransactionCommit() != NO_ERROR)
	{
		LOGFILE("Hook Transcation Commit Failed");	
		exit(-1);
	}
	
}

//------------------------------------------------------------------------------------------
//	Function name:	UnhookAPI
//	Description:	Unhooking the real functions and our intercepted ones, upon exit.
//------------------------------------------------------------------------------------------
void UnhookAPI()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&(PVOID&)Real_Direct3DCreate8, Mine_Direct3DCreate8);
	//DetourDetach(&(PVOID&)Real_CreateWindowExA,Mine_CreateWindowExA) ;
	//DetourAttach(&(PVOID&)Real_CreateWindowExW,Mine_CreateWindowExW) ;
	DetourTransactionCommit();
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