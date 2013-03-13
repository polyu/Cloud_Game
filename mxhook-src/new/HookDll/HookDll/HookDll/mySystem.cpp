#pragma once

#include "stdafx.h"
#include "mySystem.h"

//-------------------------------------------------------------
_MessageBoxA TrueMessageBoxA = (_MessageBoxA)::MessageBoxA;

int WINAPI HookMessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) 
{
	OutputDebugStringA("HookMessageBoxA---------------");

	return TrueMessageBoxA(hWnd, lpText, lpCaption, uType);
}

//-------------------------------------------------------------
_CreateProcessInternalW TrueCreateProcessInternalW = (_CreateProcessInternalW)GetProcAddress(\
		GetModuleHandle(TEXT("kernel32.dll")), "CreateProcessInternalW");

BOOL
WINAPI
HookCreateProcessInternalW(
	HANDLE hToken,
	LPCWSTR lpApplicationName,
	LPWSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCWSTR lpCurrentDirectory,
	LPSTARTUPINFOW lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation,
	PHANDLE hNewToken
	)
{
	OutputDebugString(L"Call MY_CreateProcessInternalW Success.\r\n");

	//TrueCreateProcessInternalW(
	//	hToken, 
	//	lpApplicationName, 
	//	lpCommandLine, 
	//	lpProcessAttributes, 
	//	lpThreadAttributes, 
	//	bInheritHandles,
	//	dwCreationFlags,
	//	lpEnvironment,
	//	lpCurrentDirectory,
	//	lpStartupInfo,
	//	lpProcessInformation,
	//	hNewToken
	//	);

	return TRUE;
}

BOOL
InitSys()
{
	if (Mhook_SetHook((PVOID*)&TrueMessageBoxA, HookMessageBoxA))
	{
		OutputDebugStringA("Mhook_SetHook TrueMessageBoxA OK---------------");
	}

	if (Mhook_SetHook((PVOID*)&TrueCreateProcessInternalW, HookCreateProcessInternalW))
	{
		OutputDebugStringA("Mhook_SetHook CreateProcessInternalW OK---------------");
	}

	return TRUE;
}

BOOL
UnInitSys()
{
	OutputDebugStringA("Mhook_Unhook TrueMessageBoxA OK---------------");

	Mhook_Unhook((PVOID*)&TrueMessageBoxA);
	Mhook_Unhook((PVOID*)&TrueCreateProcessInternalW);

	return TRUE;
}