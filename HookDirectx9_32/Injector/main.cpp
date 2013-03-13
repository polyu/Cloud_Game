//------------------------------------------------------------------------------------------
//	This Injector launch an application with dxhook injected to it.
//------------------------------------------------------------------------------------------
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <atlbase.h>
#include <atlstr.h>
#include <conio.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;
#include <detours.h>

int main(int argc,char **argv)
{
	if(argc!=3)
	{
		exit(-1);
	}
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(STARTUPINFO));
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	char applicationPath[MAX_PATH];
	char applicationName[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, applicationPath);
	GetCurrentDirectoryA(MAX_PATH, applicationName);
	strcat_s(applicationName,MAX_PATH/2,"//");
	strcat_s(applicationName,MAX_PATH/2,argv[1]);
	printf("%s\n",applicationPath);
	BOOL bResult = DetourCreateProcessWithDllA(applicationName, NULL, 0, 0, TRUE,
		CREATE_DEFAULT_ERROR_MODE , NULL,
		applicationPath,(LPSTARTUPINFOA)&si, &pi, argv[2] ,NULL);

	printf("Attached is %s.\n", (bResult)?"successful":"unsuccessful");
	
}

