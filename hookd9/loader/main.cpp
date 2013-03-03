#include "..\\scwlapi\\commdef-int.h"
#include "..\\detours\\detours.h"


int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	PROCESS_INFORMATION pi = {0};
	STARTUPINFO si = {0};
	si.cb = sizeof(si);

	BOOL ret = 
	DetourCreateProcessWithDllW(
		L"Blobs.exe",
		NULL,
		NULL,
		NULL,
		TRUE,
		CREATE_DEFAULT_ERROR_MODE,
		NULL,
		NULL,
		&si,
		&pi,
		NULL,
		"blob.dll",
		NULL);

	if(!ret)
	{
		MessageBoxW(NULL, L"Ü¢Ñ”Blobs.exe ßî°!", L"", MB_OK);
	}

	return 0;
}