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
using std::cout;
using std::string;
using std::wstring;
using std::ifstream;
using std::endl;

#pragma comment(lib,"..\\Detours\\detours.lib")
#include "..\Detours\detours.h"

//	Forward Deceleration
std::wstring s2ws(const std::string& s);

int main()
{

	STARTUPINFO si = {sizeof(STARTUPINFO)};
	PROCESS_INFORMATION pi = {0};

	//	Reading parameters from the text file
	ifstream file("Intercept.txt");
	string exeFullpath;
	string exePath;

	char tmp[256];
	file.getline(tmp, 256);
	wstring gameStr(s2ws(tmp));
	file.getline(tmp, 256);
	wstring gamePath(s2ws(tmp));

	////////////////////////////////////
	// Launch The Process
	////////////////////////////////////
	BOOL bResult = DetourCreateProcessWithDll(gameStr.c_str(), NULL, 0, 0, TRUE,
		CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_CONSOLE, NULL,
		gamePath.c_str(),&si, &pi, L"dxhook.dll" , 0 );

	//	Closing injector
	file.close();

	printf("Attached is %s.\n", (bResult)?"successful":"unsuccessful");
	if (!bResult)
		getch();
}

//	this function converts a string to a wstring (for windows API needs)
std::wstring s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}