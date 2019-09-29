#include <Windows.h>
#include <tchar.h>
#include <stdio.h>

#define MAX_DIRLEN (MAX_PATH + 2)

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

int _tmain(int argc,LPTSTR argv[])
{
	LPTSTR buffer;
	TCHAR pwdBuffer[MAX_DIRLEN];
	 
	/** Retrieves the current directory for the current process.
	DWORD WINAPI GetCurrentDirectory(
	  __in   DWORD nBufferLength,
	  __out  LPTSTR lpBuffer
	);
	Multithreaded applications and shared library code should not use the GetCurrentDirectory function
	and should avoid using relative path names
	**/
	DWORD nLen = GetCurrentDirectory(256,pwdBuffer);

	if(nLen == 0)
		ReportError(_T("GetCurrentDirectory Failed"),0,FALSE);
	if(nLen >= MAX_DIRLEN)
		ReportError(_T("Dir path too long!"),0,FALSE);

	PrintMsg(GetStdHandle(STD_OUTPUT_HANDLE),pwdBuffer);


	return 0;
}