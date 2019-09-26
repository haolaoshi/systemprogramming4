//#define UNICODE 
#define _UNICODE
#include <Windows.h>

#include <tchar.h>
#include <stdio.h>
VOID ReportError(LPTSTR lpMsg,DWORD val,BOOL showInfo);

int _tmain(int argc,char** argv)
{
	ReportError("Normal Message",0,TRUE);

	return 0;
}