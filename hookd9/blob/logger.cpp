#include "iris-int.h"

#ifndef __RELEASE

using namespace std;

wofstream fs;
CRITICAL_SECTION cs;

void CreateLogger(const char* fileName)
{
	// Initialize the critical section one time only.
	InitializeCriticalSectionAndSpinCount(&cs, 0x80000400);

	fs.open(fileName, ios::out | ios::app);
}

void CloseLogger()
{
	fs.close();

    // Release resources used by the critical section object.
    DeleteCriticalSection(&cs);
}

void WriteLogger(const wchar_t* lpszText, ...)
{
	wchar_t buffer[MAX_CHAR];

	va_list ap;
	va_start(ap, lpszText);
	wvsprintf(buffer, lpszText, ap);
	va_end(ap);

	// Request ownership of the critical section.
	EnterCriticalSection(&cs); 

	fs << buffer << endl;
	fs.flush();

	// Release ownership of the critical section.
	LeaveCriticalSection(&cs);
}

#endif //__RELEASE