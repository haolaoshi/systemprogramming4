#include <Windows.h>
#include <stdarg.h>
#include <tchar.h>
#include <stdio.h>

VOID ReportError(LPTSTR lpMsg,DWORD val,BOOL showInfo){

	DWORD i  = GetLastError();
	LPTSTR buffer;

	_ftprintf(stderr,_T("lpMsg = %s"),lpMsg);//Print formatted data to a stream

	if(showInfo){

		DWORD msgLen =  FormatMessage(
		  FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,//   dwFlags,
		  NULL,// lpSource,
		  i,//   dwMessageId,
		  NULL, //  dwLanguageId,
		  (LPTSTR)&buffer,//  lpBuffer,
		  0,//   nSize,
		  NULL// *Arguments
		);

		if(msgLen > 0){
			_tprintf(_T("%s : %s\n"),lpMsg,buffer);
		}else{
			_tprintf(_T("Fatal Error when format %s : %d "),lpMsg,i);
		}
	}

	if(val > 0 ){
		ExitProcess(val);
	}
}


//key : va_list
BOOL PrintString(HANDLE hOut,...){

	DWORD msgLen,count;
	LPCTSTR pMsg;
	va_list pMsgList;//Current Msg String

	va_start(pMsgList,hOut);//Start Process Msg

	while((pMsg = va_arg(pMsgList,LPCTSTR)) != NULL){
		msgLen = _tcslen(pMsg);
		if(!WriteConsole(hOut,pMsg,msgLen,&count,NULL) && !WriteFile(hOut,pMsg,msgLen*sizeof(TCHAR),&count,NULL))
			va_end(pMsgList);
		return FALSE;
	}
	va_end(pMsgList);
	return TRUE;
}

BOOL PrintMsg(HANDLE hOut,LPCTSTR msg){

	return PrintString(hOut,msg);
}

//key : SetConsoleMode
BOOL ConsolePrompt(LPCTSTR promptMsg,LPTSTR pResponse,DWORD maxChar,DWORD echo){

	HANDLE hIn,hOut;

	DWORD charIn,echoFlag;
	BOOL success;
	hIn = CreateFile(_T("COINS$"),GENERIC_READ | GENERIC_WRITE , 0, NULL, OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	hOut = CreateFile(_T("COOUT$"),GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	
	echoFlag = echo ? ENABLE_ECHO_INPUT : 0;
	/**BOOL WINAPI SetConsoleMode(
	  __in  HANDLE hConsoleHandle,
	  __in  DWORD dwMode
	);
	ENABLE_PROCESSED_INPUT 

	ENABLE_WRAP_AT_EOL_OUTPUT 
	**/
	success = SetConsoleMode(hIn,ENABLE_LINE_INPUT |echoFlag | ENABLE_PROCESSED_INPUT)
		&& SetConsoleMode(hOut,ENABLE_WRAP_AT_EOL_OUTPUT | ENABLE_PROCESSED_INPUT)
		&& PrintString(hOut,promptMsg,NULL)
		&& ReadConsole(hIn,pResponse,maxChar - 2, &charIn,NULL);

	if(success)
		pResponse[charIn - 2 ] = '\0';
	else 
		ReportError(_T("Console prompt Failed."),0,TRUE);
	CloseHandle(hIn);
	CloseHandle(hOut);
	return success;
}