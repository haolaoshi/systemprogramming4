#include <Windows.h>
#include <tchar.h>
#include <stdio.h>

#define BUF_SIZE	256
/**
DeleteFile  CopyFile 
CreateHardLink  CreateSymbolicLink  
MoveFile MoveFileEx 
CreateDirectory   RemoveDirectory 
SetCurrentDirectory GetCurrentDirectory 
SetConsoleMode  ReadConsole 
FreeConsole  AllocConsole 


variable-length argument list 
***/
#include <stdarg.h>

//use variable arguments list 
BOOL PrintStrings(HANDLE hOut,...){
	DWORD msgLen,count;
	LPCTSTR pMsg;
	va_list pMsgList;
	va_start(pMsgList,hOut);
	while((pMsg = va_arg(pMsgList,LPCTSTR))!= NULL){//参数表不空
		//You can use _tcslen() to get the string length.
		//For ANSI, _tcslen() returns the number of bytes. 
		//For Unicode, _tcslen() returns the number of WCHARs (that is, WORDs).
		msgLen = _tcslen(pMsg);
		//WriteConsole Successed ONLY if for console handles , Call WriteFile only if WriteConsole Fails 
		while(!WriteConsole(hOut,pMsg,msgLen,&count,NULL)&& !WriteFile(hOut,pMsg,msgLen*sizeof(TCHAR),&count,NULL) && count){
			va_end(pMsgList);
			return FALSE;//WRITE CONSOLE && WRITE FILE ALL FAILS, 
		}//while -> if 
	}
	va_end(pMsgList);
	return TRUE;
}

BOOL PrintMsg(HANDLE hOut,LPCTSTR pMsg){
	//Single message version of PrintMsg 
	return PrintStrings(hOut,pMsg,NULL);
}

VOID ReportError(LPCTSTR pMsg,DWORD ret,BOOL show){
	DWORD i = GetLastError();
	LPTSTR buffer;
	_ftprintf(stderr,_T(" ReportError %s \n"),pMsg);

	if(show){
		DWORD msgLen = FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,//   dwFlags,
			NULL, // lpSource,
			i ,  //   dwMessageId,
			NULL,
			(LPTSTR)&buffer,
			0,
			NULL);
		if(msgLen > 0)
			_ftprintf(stderr,_T(" %s "),buffer);

		else
			_tprintf(_T("Fatal Error on %s ; %d "),pMsg,i);

	}
	if(ret > 0){
		ExitProcess(ret);
	}
	return ;
}
BOOL ConsolePrompt(LPCTSTR prompt,LPTSTR pResponse, DWORD nMax,BOOL echo){
	HANDLE hIn,hOut;

	DWORD charIn,echoFlag;
	BOOL success;
	//-----------------------------------------------------
	/***#define CREATE_NEW          1
		#define CREATE_ALWAYS       2
		#define OPEN_EXISTING       3
		#define OPEN_ALWAYS         4
		#define TRUNCATE_EXISTING   5
	****/
	hIn = CreateFile(_T("CONIN$"),GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	hOut = CreateFile(_T("CONOUT$"),GENERIC_WRITE,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	/**
	//
//  Input Mode flags:	
//

	#define ENABLE_PROCESSED_INPUT  0x0001
	#define ENABLE_LINE_INPUT       0x0002
	#define ENABLE_ECHO_INPUT       0x0004
	#define ENABLE_WINDOW_INPUT     0x0008
	#define ENABLE_MOUSE_INPUT      0x0010
	#define ENABLE_INSERT_MODE      0x0020
	#define ENABLE_QUICK_EDIT_MODE  0x0040
	#define ENABLE_EXTENDED_FLAGS   0x0080
	#define ENABLE_AUTO_POSITION    0x0100
	**/
	echoFlag = echo ? ENABLE_ECHO_INPUT : 0;
	/**
	//
//  Input Mode flags:
//

	#define ENABLE_PROCESSED_INPUT  0x0001
	#define ENABLE_LINE_INPUT       0x0002
	#define ENABLE_ECHO_INPUT       0x0004
	#define ENABLE_WINDOW_INPUT     0x0008
	#define ENABLE_MOUSE_INPUT      0x0010
	#define ENABLE_INSERT_MODE      0x0020
	#define ENABLE_QUICK_EDIT_MODE  0x0040
	#define ENABLE_EXTENDED_FLAGS   0x0080
	#define ENABLE_AUTO_POSITION    0x0100

	**/
	success = SetConsoleMode(hIn,ENABLE_LINE_INPUT | echoFlag | ENABLE_PROCESSED_INPUT)
		&& SetConsoleMode(hOut,ENABLE_WRAP_AT_EOL_OUTPUT|ENABLE_PROCESSED_OUTPUT)
		&& PrintStrings(hOut,prompt,NULL)
		&& ReadConsole(hIn,pResponse,nMax - 2,&charIn,NULL);
	//Replace CR-LF by the null character **/
	if(success)
		pResponse[charIn - 2] = '\0';
	else 
		ReportError(_T("console prompt error"),0,FALSE);
	CloseHandle(hIn);
	CloseHandle(hOut);

	return success;
}

#define MAX_PATH 256
int _tmain(int argc , PTSTR argv[])
{
	TCHAR buffer[MAX_PATH];
	int ret = GetCurrentDirectory(MAX_PATH,buffer);

	if(ret >= MAX_PATH)
		ReportError(_T("Failure get Dir name !!"),1,TRUE);

	if(ret == 0)
		ReportError(_T("Too long... "),2	,TRUE);

	PrintMsg(GetStdHandle(STD_OUTPUT_HANDLE),buffer);
	 
	return 0;
}
