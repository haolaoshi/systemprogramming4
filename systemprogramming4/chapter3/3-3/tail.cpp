#include <Windows.h>
#include <tchar.h>

#define NUM_LINES 11
	/* One more than number of lines in the tail. */
#define MAX_LINE_SIZE 256
#define MAX_CHAR NUM_LINES*MAX_LINE_SIZE
 

#include <stdarg.h>

VOID ReportError(const wchar_t  *lpMsg,DWORD val,BOOL showInfo){

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
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <io.h>

int _tmain (int argc, LPTSTR argv [])
{
	HANDLE hInFile;
	HANDLE hStdOut = GetStdHandle (STD_OUTPUT_HANDLE);
	LARGE_INTEGER FileSize, CurPtr;
	LARGE_INTEGER LinePos [NUM_LINES];
	DWORD LastLine, FirstLine, LineCount, nRead, FPos;
	TCHAR Buffer [MAX_CHAR + 1], c;

	if (argc != 2)
		ReportError (_T ("Usage: tail file"), 1, FALSE);
	hInFile = CreateFile (argv [1], GENERIC_READ,
			0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hInFile == INVALID_HANDLE_VALUE)
		ReportError (_T ("tail error: Cannot openfile."), 2, TRUE);

	/* Get the current file size. */

	FileSize.LowPart = GetFileSize (hInFile, &FileSize.HighPart);
	if (FileSize.LowPart == 0xFFFFFFFF && GetLastError () != NO_ERROR)
		ReportError (_T ("tail error: file size"), 3, TRUE);

	/* Set the file pointer on the assumption that 256
		is the maximum size and look for the line beginnings.
		If 10 lines are not located, then just put out the
		ones that are found. A more general solution would look
		farther back in the file if necessary. Alternatively,
		start the scan at the beginning of the file, but that
		would be time consuming for a large file. */

	CurPtr.QuadPart = (LONGLONG)FileSize.QuadPart
			- NUM_LINES * MAX_LINE_SIZE * sizeof (TCHAR);
	if (CurPtr.QuadPart < 0) CurPtr.QuadPart = 0;
	FPos = SetFilePointer (hInFile, CurPtr.LowPart, &CurPtr.HighPart, FILE_BEGIN);
	if (FPos == 0xFFFFFFFF && GetLastError () != NO_ERROR)
		ReportError (_T ("tail Error: Set Pointer."), 4, TRUE);

	/*  Scan the file for the start of new lines and retain their
		position. Assume that a line starts at the current position. */

	LinePos [0].QuadPart = CurPtr.QuadPart;
	LineCount = 1;
	LastLine = 1;
	while (TRUE) {
		while (ReadFile (hInFile, &c, sizeof (TCHAR), &nRead, NULL)
				&& nRead > 0 && c != CR) ; /* Empty loop body */
		if (nRead < sizeof (TCHAR)) break;
					/* Found a CR. Is LF next? */
		ReadFile (hInFile, &c, sizeof (TCHAR), &nRead, NULL);
		if (nRead < sizeof (TCHAR)) break;
		if (c != LF) continue;
		CurPtr.QuadPart = 0;
				/* Get the current file position. */
		CurPtr.LowPart = SetFilePointer (
				hInFile, 0, &CurPtr.HighPart, FILE_CURRENT);
				/* Retain the start-of-line position */
		LinePos [LastLine].QuadPart = CurPtr.QuadPart;
		LineCount++;
		LastLine = LineCount % NUM_LINES;
	}
	
	FirstLine = LastLine % NUM_LINES;
	if (LineCount < NUM_LINES) FirstLine = 0;
	CurPtr.QuadPart = LinePos [FirstLine].QuadPart;

	/* The start of each line is now stored in LinePos []
		with the last line having index LastLine.
		Display the last strings. */

	SetFilePointer (hInFile, CurPtr.LowPart, &CurPtr.HighPart, FILE_BEGIN);
	ReadFile (hInFile, Buffer, sizeof (Buffer), &nRead, NULL);
	Buffer [nRead] = '\0';
	PrintMsg (hStdOut, Buffer);
	CloseHandle (hInFile);
	return 0;
}


