/*************************************************************************

Framework\commdef-int.h

	-by Miles Chen (stainboyx@hotmail.com) 2008-12-27

*************************************************************************/


#ifndef MAX_CHAR
#define MAX_CHAR		256
#endif

#ifndef SHORT_CHAR
#define SHORT_CHAR		32
#endif




#include <Ws2tcpip.h>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <exdisp.h>
#include <mshtml.h>
#include <crtdbg.h>		/* for _ASSERT() */
#include <psapi.h>

#pragma comment (lib, "psapi.lib")
#pragma comment (lib, "comctl32.lib")
#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "shlwapi.lib")
#pragma comment (lib, "Version.lib")


#define N namespace


#include "System.Diagnostics.Process.h"
#include "System.Runtime.InteropServices.Marshal.h"
#include "System.Security.Injection.h"
#include "System.String.h"



using namespace System;
using namespace System::Diagnostics;
using namespace System::Runtime::InteropServices;
using namespace System::Security;

