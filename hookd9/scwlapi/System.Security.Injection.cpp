/*************************************************************************

Framework\System.Security.Injection.cpp

	-by Miles Chen (stainboyx@hotmail.com) 2008-12-27

*************************************************************************/

#pragma once

#include "commdef-int.h"

N System{N Security{N Injection{



	BOOL InjectLibrary(HANDLE hProcess, LPCWSTR lpszDllFullpath)
	{
		const DWORD dwMaxWaitTime = 5000;
		SIZE_T dwSize;
		LPVOID lpStartAddress = NULL;
		LPVOID lpParameter = NULL;
		HANDLE hThread = NULL;
		DWORD dwResult;
		BOOL result;

		dwSize = String::Length(lpszDllFullpath) * sizeof(WCHAR);
		lpStartAddress = LoadLibraryW;
		lpParameter = VirtualAllocEx(hProcess, NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
		if(!lpParameter)
		{
			result = FALSE;
			goto exit;
		}

		if(!WriteProcessMemory(hProcess, lpParameter, lpszDllFullpath, dwSize, NULL))
		{
			result = FALSE;
			goto exit;
		}

		hThread = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)lpStartAddress, lpParameter, NULL, NULL);    
		if(!hThread)
		{
			result = FALSE;
			goto exit;
		}

		dwResult = WaitForSingleObject(hThread, dwMaxWaitTime);
		switch(dwResult)
		{
		case WAIT_OBJECT_0:
			{
				// this might just have worked, pick up the result!
				result = TRUE;
			}
			break;
		case WAIT_TIMEOUT:
		case WAIT_FAILED:
		default:
			{
				result = FALSE;
				goto exit;
			}
		}

exit:
		if(lpParameter)
			VirtualFreeEx(hProcess, lpParameter, 0, MEM_RELEASE);
		if(hThread)
			CloseHandle(hThread);

		return result;
	}

	BOOL InjectLibrary(DWORD dwProcID, LPCWSTR lpszDllFullpath)
	{
		BOOL result;
		HANDLE hProcess = NULL;

		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcID);
		if(!hProcess)
		{
			result = FALSE;
			goto exit;
		}

		result = InjectLibrary(hProcess, lpszDllFullpath);

exit:
		if(hProcess)
			CloseHandle(hProcess);
		return result;
	}

	BOOL InjectLibrary(HWND hWnd, LPCWSTR lpszDllFullpath)
	{
		DWORD dwProcessID;

		GetWindowThreadProcessId(hWnd, &dwProcessID);

		return InjectLibrary(dwProcessID, lpszDllFullpath);
	}

	BOOL EnableDebugPrivilege()
	{
		HANDLE hToken;   
		if( !OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken ) )   
		{
			return FALSE;
		}

		TOKEN_PRIVILEGES tp;   
		tp.PrivilegeCount = 1;   
		if( !LookupPrivilegeValueW( NULL, SE_DEBUG_NAME, &tp.Privileges[ 0 ].Luid ) )
		{
			return FALSE;
		}

		tp.Privileges[ 0 ].Attributes = SE_PRIVILEGE_ENABLED;   
		if( !AdjustTokenPrivileges( hToken, FALSE, &tp, sizeof(tp), NULL, NULL ) )
		{
			return FALSE;
		}

		CloseHandle( hToken );
		return TRUE;	
	}


}}}