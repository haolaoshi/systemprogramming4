//#define UNICODE 
#define _UNICODE
#include <Windows.h>

#include <tchar.h>
#include <stdio.h>
VOID ReportError(LPTSTR lpMsg,DWORD val,BOOL showInfo);
//show Generic printf & scanf usage:

int _tmain(int argc,char** argv)
{
	TCHAR name[256];
 
	_tprintf(_T("PLEASE INPUT YOUR NAME:"));
	_tscanf(_T("%s"),&name);
	_tprintf(_T("Welcome %s\n"),name);
	ReportError("Normal Message",0,TRUE);

	return 0;
}