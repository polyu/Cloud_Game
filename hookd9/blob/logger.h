

#ifdef __RELEASE

#define CreateLogger(...)
#define CloseLogger(...)
#define WriteLogger(...)

#else

void CreateLogger(const char* fileName);
void CloseLogger();
void WriteLogger(const wchar_t* lpszText, ...);

#endif //__RELEASE


