#define UNICODE
#define _UNICODE

#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include <stdarg.h>

#ifdef _UNICODE
  #define _memtchr wmemchr
#else
  #define _memtchr memchr
#endif
#define BUF_SIZE 256


 VOID CatFile(HANDLE,HANDLE);
DWORD Options (int argc, LPCTSTR argv [], LPCTSTR OptStr, ...);


VOID ReportError(LPTSTR lpMsg,DWORD val,BOOL showInfo);