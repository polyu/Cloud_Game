/*************************************************************************

Framework\System.Diagnostics.Process.cpp

	-by Miles Chen (stainboyx@hotmail.com) 2008-12-27

*************************************************************************/

#pragma once

#include "commdef-int.h"

N System{N Diagnostics{N Process{


#define MAX_MODULE 1024

	BOOL Start(LPCWSTR lpFile, LPWSTR lpCommandLine, LPPROCESS_INFORMATION ppi)
	{
		STARTUPINFO si = {0};
		si.cb = sizeof(si);

		return CreateProcessW(
			lpFile,
			lpCommandLine,
			NULL,
			NULL,
			TRUE,
			CREATE_DEFAULT_ERROR_MODE,
			NULL,
			NULL,
			&si,
			ppi);
	}


	BOOL StartWithInjection(LPCWSTR lpFile, LPWSTR lpCommandLine, LPCWSTR lpszLib, LPPROCESS_INFORMATION ppi)
	{
		if(!ppi)
		{
			PROCESS_INFORMATION pi;
			ppi = &pi;
		}

		if(!Start(lpFile, lpCommandLine, ppi))
		{
			return FALSE;
		}

		return Injection::InjectLibrary(ppi->hProcess, lpszLib);
	}

	BOOL Start(LPCWSTR lpFile, LPPROCESS_INFORMATION ppi)
	{
		return Start(lpFile, NULL, ppi);
	}

	BOOL Kill(HANDLE hProcess)
	{
		return TerminateProcess(hProcess, -1);
	}

	BOOL GetProcessFileName(DWORD dwProcID, LPWSTR lpszFileName, int nSize)
	{
		BOOL result = TRUE;
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcID);
		if(!hProcess)
		{
			result = FALSE;
			goto exit;
		}

		if(!GetProcessFileName(hProcess, lpszFileName, nSize))
		{
			result = FALSE;
			goto exit;
		}

exit:
		if(hProcess)
			CloseHandle(hProcess);
		return result;
	}

	BOOL GetProcessFileName(HANDLE hProcess, LPWSTR lpszFileName, int nSize)
	{
		HMODULE hMods[MAX_MODULE];
		DWORD cb;
		BOOL ret = EnumProcessModules(hProcess, hMods, MAX_MODULE, &cb);
		ret &= GetModuleFileNameExW(hProcess, hMods[0], lpszFileName, nSize) != 0;

		return ret;
	}

	BOOL GetProcessBaseName(HANDLE hProcess, LPWSTR lpszBaseName, int nSize)
	{
		HMODULE hMods[MAX_MODULE];
		DWORD cb;
		BOOL ret = EnumProcessModules(hProcess, hMods, MAX_MODULE, &cb);
		ret &= GetModuleBaseNameW(hProcess, hMods[0], lpszBaseName, nSize) != 0;

		return ret;
	}

	BOOL InternalEnumProcessModules(HANDLE hProcess, DWORD nSize, MODENUMPROC lpEnumFunc, LPARAM lParam, LONG lpSpecificFunc)
	{
		HMODULE* hMods = (HMODULE*)Marshal::AllocCoTaskMem(sizeof(HMODULE));
		LPWSTR lpszModName = (LPWSTR)Marshal::AllocCoTaskMem(nSize);
		DWORD cb;
		BOOL fRet;

		fRet = EnumProcessModules(hProcess, hMods, sizeof(HMODULE), &cb);
		if(!fRet)
			goto __exit;

		hMods = (HMODULE*)Marshal::ReAllocCoTaskMem(hMods, cb);
		fRet = EnumProcessModules(hProcess, hMods, cb, &cb);
		if(!fRet)
			goto __exit;

		for(int i=0; i<(int)(cb / sizeof(HMODULE)); i++)
		{
			fRet &= ((GETMODULECALL)lpSpecificFunc)(hProcess, hMods[i], lpszModName, nSize) != 0;
			fRet &= lpEnumFunc(lpszModName, lParam);

			if(!fRet)
				goto __exit;
		}

__exit:
		Marshal::FreeCoTaskMem(hMods);
		Marshal::FreeCoTaskMem(lpszModName);
		return fRet;
	}

	BOOL EnumProcessModuleFileNames(HANDLE hProcess, DWORD nSize, MODENUMPROC lpEnumFunc, LPARAM lParam)
	{
		return InternalEnumProcessModules(hProcess, nSize, lpEnumFunc, lParam, (LONG)GetModuleFileNameExW);
	}
	BOOL EnumProcessModuleBaseNames(HANDLE hProcess, DWORD nSize, MODENUMPROC lpEnumFunc, LPARAM lParam)
	{
		return InternalEnumProcessModules(hProcess, nSize, lpEnumFunc, lParam, (LONG)GetModuleBaseNameW);
	}

}}}