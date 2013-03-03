/*************************************************************************

Framework\System.Diagnostics.Process.h

	-by Miles Chen (stainboyx@hotmail.com) 2008-12-27

*************************************************************************/

N System{N Diagnostics{N Process{


	typedef BOOL (CALLBACK* MODENUMPROC)(LPCWSTR, LPARAM);
	typedef DWORD (WINAPI* GETMODULECALL)(
		__in HANDLE hProcess,
		__in_opt HMODULE hModule,
		__out_ecount(nSize) LPWSTR lpszModName,
		__in DWORD nSize
		);

	BOOL Start(LPCWSTR lpFile, LPWSTR lpCommandLine, LPPROCESS_INFORMATION ppi);

	BOOL Start(LPCWSTR lpFile, LPPROCESS_INFORMATION ppi);

	BOOL Kill(HANDLE hProcess);

	BOOL GetProcessFileName(DWORD dwProcID, LPWSTR lpszFileName, int nSize);
	BOOL GetProcessFileName(HANDLE hProcess, LPWSTR lpszFileName, int nSize);

	BOOL GetProcessBaseName(HANDLE hProcess, LPWSTR lpszBaseName, int nSize);

	BOOL StartWithInjection(LPCWSTR lpFile, LPWSTR lpCommandLine, LPCWSTR lpszLib, LPPROCESS_INFORMATION ppi);

	BOOL EnumProcessModuleFileNames(HANDLE hProcess, DWORD nSize, MODENUMPROC lpEnumFunc, LPARAM lParam);

	BOOL EnumProcessModuleBaseNames(HANDLE hProcess, DWORD nSize, MODENUMPROC lpEnumFunc, LPARAM lParam);

}}}