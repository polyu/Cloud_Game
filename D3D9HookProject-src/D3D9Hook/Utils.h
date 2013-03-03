#pragma once

// COM utils
template<class COMObject>
void SafeRelease(COMObject*& pRes)
{
    IUnknown *unknown = pRes;
    if (unknown)
    {
        unknown->Release();
    }
    pRes = NULL;
}

//util for getting OS version
typedef enum OSVersion
{
    WIN95 = 1,
    WIN98 = 2,
    WINME = 3,
    WINNT351 = 4,
    WINNT4 = 5,
    WIN2000 = 6,
    WINXP = 7,
    WIN2003 = 8,
    WINCE = 9,
    WINLH = 10,
    WIN64BIT = 11
} OS_VERSION;

OS_VERSION GetHostOS()
{
    BOOL bIs64BitOS = FALSE;
    BOOL bIsWin95 = FALSE;
    BOOL bIsWin98 = FALSE;
    BOOL bIsWinME = FALSE;
    BOOL bIsWinNT351 = FALSE;
    BOOL bIsWinNT4 = FALSE;
    BOOL bIsWin2000 = FALSE;
    BOOL bIsWinXP = FALSE;
    BOOL bIsWin2003 = FALSE;
    BOOL bIsWinLH = FALSE;
    
    // We check if the OS is 64 Bit
    typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

    LPFN_ISWOW64PROCESS 
    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
    GetModuleHandle("kernel32"),"IsWow64Process");
 
    if (NULL != fnIsWow64Process)
    {
        if (!fnIsWow64Process(GetCurrentProcess(),&bIs64BitOS))
        {
            // handle error?
        }
    }

    OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    ::GetVersionEx( &osvi );
    
    bIsWin95 =
       (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) &&
       (osvi.dwMajorVersion == 4) &&
       (osvi.dwMinorVersion == 0);

    bIsWin98 =
       (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) &&
       (osvi.dwMajorVersion == 4) &&
       (osvi.dwMinorVersion == 10);

    bIsWinME = 
       (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) &&
       (osvi.dwMajorVersion == 4) &&
       (osvi.dwMinorVersion == 90); 

    bIsWinNT351 = 
       (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
       (osvi.dwMajorVersion == 3) &&
       (osvi.dwMinorVersion == 51);

    bIsWinNT4 = 
       (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
       (osvi.dwMajorVersion == 4) &&
       (osvi.dwMinorVersion == 0);

    bIsWin2000 = 
       (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
       (osvi.dwMajorVersion == 5) &&
       (osvi.dwMinorVersion == 0);

    bIsWinXP = 
       (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
       (osvi.dwMajorVersion == 5) &&
       (osvi.dwMinorVersion == 1);

    bIsWin2003 = 
       (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
       (osvi.dwMajorVersion == 5) &&
       (osvi.dwMinorVersion == 2);

    bIsWinLH = 
        (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
        (osvi.dwMajorVersion == 6);

	if( bIs64BitOS )
	{
		return WIN64BIT;
	}

    if( bIsWin95 ) return WIN95;
    if( bIsWin98 ) return WIN98;
    if( bIsWinME ) return WINME;
    if( bIsWinNT351 ) return WINNT351;
    if( bIsWinNT4 ) return WINNT4;
    if( bIsWin2000 ) return WIN2000;
    if( bIsWinXP ) return WINXP;
    if( bIsWin2003 ) return WIN2003;
    if( bIsWinLH ) return WINLH;

    return WINXP;
}

