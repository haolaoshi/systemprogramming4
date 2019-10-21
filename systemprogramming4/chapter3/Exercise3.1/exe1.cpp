#define _UNICODE
#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
//Determine if files are stored sparsely or if all space is consumed

void ReportSpace (LPCTSTR Message);

VOID ReportError(const wchar_t  *lpMsg,DWORD val,BOOL showInfo);

int _tmain(int argc,LPCSTR argv[])
{
	HANDLE hFile;
	LARGE_INTEGER FileLen, FileLenH;
	BYTE Buffer [256];
	OVERLAPPED ov;
	DWORD nWrite;

	while (TRUE) {
		FileLen.QuadPart = 0;
		_tprintf (_T("Enter file length (Zero to quit): "));
		_tscanf (_T("%I64d"), &FileLen.QuadPart);
		_tprintf (_T("Length: %d\n"), FileLen.QuadPart);
		/* Note: this does not really handle numbers bigger than 4G. = 4294967296
			Can you fix it? */
		if (FileLen.QuadPart== 0) break;
		FileLenH.QuadPart = FileLen.QuadPart/2;

		ReportSpace (_T("Before file creation"));

		hFile = CreateFile (_T("TempTestFile"), GENERIC_READ|GENERIC_WRITE, 0, NULL,
			CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			ReportError (_T("Cannot create TempTestFile"), 2, TRUE);
		ReportSpace (_T("After file creation"));

		if (SetFilePointer (hFile, FileLen.LowPart, &FileLen.HighPart, FILE_BEGIN) == 0xffffffff
			&& GetLastError() != 0)
			ReportError (_T("Cannnot set file pointer"), 3, TRUE);
		
		if (!SetEndOfFile (hFile))
			ReportError (_T("Cannot set end of file"), 4, TRUE);
		ReportSpace (_T("After setting file length"));

		ov.Offset = FileLenH.LowPart;
		ov.OffsetHigh = FileLen.HighPart;
		ov.hEvent = NULL; 

		if (!WriteFile (hFile, Buffer, sizeof(Buffer), &nWrite, &ov))
			ReportError (_T("Cannot write file in middle"), 5, TRUE);
		
		ReportSpace (_T("After writing to middle of file"));

		CloseHandle (hFile);
		DeleteFile (_T("TempTestFile"));
	}
	_tprintf (_T("End of FreeSpace"));
	return 0;
}

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

void ReportSpace (LPCTSTR Message)
{
	ULARGE_INTEGER FreeBytes, TotalBytes, NumFreeBytes;
	
	if (!GetDiskFreeSpaceEx (NULL, &FreeBytes, &TotalBytes, &NumFreeBytes))
		ReportError ((LPTSTR)("Cannot get free space"), 1, TRUE);
	/* Note: Windows NT 5.0 and greater (including 2000) - This measures
	space available to the user, accounting for disc quotas */
	_tprintf (_T("%35s.\nTotal: %16x; Free on disk: %16x; Avail to user: %16x\n"), Message,
		TotalBytes.QuadPart, NumFreeBytes.QuadPart, FreeBytes.QuadPart);

}
