/*************************************************************************

Framework\System.Security.Injection.h

	-by Miles Chen (stainboyx@hotmail.com) 2008-12-27

*************************************************************************/

N System{N Security{N Injection{

	BOOL InjectLibrary(HANDLE hProcess, LPCWSTR lpszDllFullpath);

	BOOL InjectLibrary(DWORD dwProcID, LPCWSTR lpszDllFullpath);

	BOOL InjectLibrary(HWND hWnd, LPCWSTR lpszDllFullpath);

	BOOL EnableDebugPrivilege();


}}}