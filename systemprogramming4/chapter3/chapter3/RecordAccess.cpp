/* Chapter 3 RecordAccess.*/
/* Usage: RecordAccess FileName [nrec]
	if nrec is omitted,FileName already exist.
	if nrec > 0, FileName is recreated (destroying any existring file)
		and the program exists , having created an empty file.
	if the number of records is large, a sparse file is recommened.
*/

/*	This program illustrates:
	1,	Random file access.
	2,	LARGE_INTEGER arithmetic and using 64bit file positions
	3,	Record update in place.
	4,	File inititialization to 0
**/
/**

typedef union _LARGE_INTEGER {
  struct {
    DWORD LowPart;
    LONG  HighPart;
  };
  struct {
    DWORD LowPart;
    LONG  HighPart;
  } u;
  LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;
**/
#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
 

#define STRING_SIZE 256

//308 byte = 256 + 4 + 16*3 = 308
typedef struct _RECORD { //File Record Structure
	DWORD	referenceCount;		//0 means a empty record, 4byte
	SYSTEMTIME	recrodCreationTime; //WORD= 2 byte, 16
	SYSTEMTIME	recordLastRefernceTime;
	SYSTEMTIME	recordUpdateTime;            
	//pass 52byte then meet contents:
	TCHAR		dataString[STRING_SIZE];  //256
} RECORD;
//8 byte
typedef struct	_HEADER{ //File header Descriptor
	DWORD	numRecords; // 03 E8 00 00  : 1000  HIGH ORDER 
	DWORD	numNonEmptyRecords;//01 00 00 00 : 
} HEADER;
 
int _tmain(int argc,char* argv[])
{
	printf("%d",sizeof(SYSTEMTIME));

	HANDLE hFile;
	LARGE_INTEGER curPtr;
	DWORD openOption,nXfer,recNo;
	RECORD record;
	TCHAR string[STRING_SIZE],command,extra;
	OVERLAPPED ov = {0,0,0,0,NULL},ovZero = {0,0,0,0,NULL};
	HEADER header = {0,0};
	SYSTEMTIME currentTime;
	BOOLEAN headerChange,recordChange;

	openOption = ((argc > 2 && atoi(argv[2]) <= 0) || argc <= 2) ? OPEN_EXISTING : CREATE_ALWAYS;
	/**
	If you pass the FILE_FLAG_SEQUENTIAL_SCAN flag, then the cache manager alters its behavior in two ways: 
	First, the amount of prefetch is doubled 
	compared to what it would have been if you hadn¡¯t passed the flag. 
	Second, the cache manager marks as available for re-use those cache pages 
	which lie entirely behind the current file pointer (assuming there are no other applications using the file). 
	
	After all, by saying that you are accessing the file sequentially, you¡¯re promising that the file pointer will always move forward.


	At the opposite extreme is FILE_FLAG_RANDOM_ACCESS. 
	In the random access case, the cache manager performs no prefetching, and it does not aggressively evict pages that lie behind the file pointer.
	Those pages (as well as the pages that lie ahead of the file pointer which you already read from or wrote to) 
	will age out of the cache according to the usual LRU policy, 
	which means that heavy random reads against a file will not pollute the cache (the new pages will replace the old ones).

	**/
	hFile = CreateFile(argv[1],GENERIC_READ|GENERIC_WRITE,0,NULL,openOption,FILE_FLAG_RANDOM_ACCESS,NULL);

	if(argc >= 3 && atoi(argv[2]) > 0){//Write the header and pre-size the file
		header.numRecords = atoi(argv[2]);
		WriteFile(hFile,&header,sizeof(header),&nXfer,&ovZero);
		curPtr.QuadPart = (LONGLONG)sizeof(RECORD) * atoi(argv[2]) + sizeof(HEADER);
		/**
		BOOL SetFilePointerEx(
		  HANDLE         hFile,
		  LARGE_INTEGER  liDistanceToMove,
		  PLARGE_INTEGER lpNewFilePointer,
		  DWORD          dwMoveMethod
		);**/

		int a = SetFilePointerEx(hFile,curPtr,NULL,FILE_BEGIN);
		if(a != 0) printf("success");
		/**
		Sets the physical file size for the specified file to the current position of the file pointer.

		The physical file size is also referred to as the end of the file. 
		The SetEndOfFile function can be used to truncate or extend a file. 
		To set the logical end of a file, use the SetFileValidData function.
		**/
		SetEndOfFile(hFile);
		return 0;
	}
	//Read file header : find number of records && non-empty records
	ReadFile(hFile,&header, sizeof(HEADER),&nXfer,&ovZero);
	//Prompt the user to read or write a numbered record 
	while(TRUE){
		headerChange = FALSE;
		recordChange = FALSE;
		_tprintf(_T("Enter r(ead)/w(write)/d(elete)/q(uit) Rec#\n"));
		_tscanf(_T("%c%u%c"),&command,&recNo,&extra);
		if(command == 'q') break;

		if(recNo >= header.numRecords){
			_tprintf(_T("Record Number is too large . Try again\n"));
			continue ;
		}
		curPtr.QuadPart = (LONGLONG)recNo * sizeof(RECORD) + sizeof(HEADER);

		/**
		The Offset and OffsetHigh members together represent a 64-bit file position. 
		It is a byte offset from the start of the file or file-like device, 
		and it is specified by the user; the system will not modify these values. 
		The calling process must set this member before passing the OVERLAPPED structure to functions that use an offset, 
		such as the ReadFile or WriteFile (and related) functions.

		**/
		ov.Offset = curPtr.LowPart;
		ov.OffsetHigh = curPtr.HighPart;
		ReadFile(hFile,&record,sizeof(RECORD),&nXfer,&ov);

		GetSystemTime(&currentTime);

		record.recordLastRefernceTime = currentTime;
		if(command == 'r' || command == 'd'){
			if(record.referenceCount == 0){
				_tprintf(_T("Record Numebr %d is empty.\n"),recNo);
				continue;
			}else{
				_tprintf(_T("Record Number %d . Reference Count : %d \n"),
					recNo,record.referenceCount);
				_tprintf(_T("Data : %s \n"),record.dataString);
				/**Exercise: dis times . 3-2**/
			}
			if(command == 'd'){
				record.referenceCount = 0;
				header.numNonEmptyRecords--;
				headerChange = TRUE;
				recordChange = TRUE;
			}
		}else if(command == 'w') {
			_tprintf(_T("Enter new data string for the record.\n"));
			_fgetts(string,sizeof(string),stdin);
			/**don'ts use _getts( overflow) **/
			string[_tcslen(string)-1] = _T('\0');
			if(record.referenceCount == 0){
				record.recrodCreationTime = currentTime;
				header.numNonEmptyRecords++;
				headerChange = TRUE;
			}
			record.recordUpdateTime = currentTime;
			record.referenceCount++;
			strncpy(record.dataString,string,STRING_SIZE-1);
			recordChange = TRUE;
		}else {
			_tprintf(_T("Command must be r,w,or d . Try again.\n"));
		}

		if(recordChange)
			WriteFile(hFile,&record,sizeof(RECORD),&nXfer,&ov);
		if(headerChange)
			WriteFile(hFile,&header,sizeof(header),&nXfer,&ovZero);
	
	}

	_tprintf(_T("Computed number of non-empty records is : %d\n"),
		header.numNonEmptyRecords);
	ReadFile(hFile,&header,sizeof(HEADER),&nXfer,&ovZero);
	_tprintf(_T("File &s has %d non-empty records.\nCapacity:%d\n"),
		argv[1],header.numNonEmptyRecords,header.numRecords);

	CloseHandle(hFile);
	return 0;
}

