#ifndef _WIN32
typedef unsigned char BYTE;
typedef unsigned  int UINT;
typedef UINT DWORD;
typedef UINT ULONG;
typedef BYTE UCHAR;
typedef char TCHAR;
typedef const char * LPCTSTR;

#else
#include <Windows.h>
#endif

#include <string>