/*************************************************************************

x509\main.cpp

		-by Miles Chen (stainboyx@hotmail.com) 2009-1-29

*************************************************************************/

#include "iris-int.h"

BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			Routine();
			break;
		}
	case DLL_THREAD_ATTACH: break;
	case DLL_THREAD_DETACH: break;
	case DLL_PROCESS_DETACH:
		{
			Cleanup();
			break;
		}
	}
	return TRUE;
}
