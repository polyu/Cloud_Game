/*************************************************************************

Framework\System.Runtime.InteropServices.Marshal.cpp

	-by Miles Chen (stainboyx@hotmail.com) 2008-12-27

*************************************************************************/

#pragma once

#include "commdef-int.h"

N System{N Runtime{N InteropServices{N Marshal{


	LPVOID AllocCoTaskMem(DWORD dwSize)
	{
		return HeapAlloc(GetProcessHeap(), 0, dwSize);
	}

	LPVOID ReAllocCoTaskMem(LPVOID lpAddress, DWORD dwSize)
	{
		return HeapReAlloc(GetProcessHeap(), 0, lpAddress, dwSize);
	}

	BOOL FreeCoTaskMem(LPVOID lpAddress)
	{
		return HeapFree(GetProcessHeap(), 0, lpAddress);
	}


}}}}