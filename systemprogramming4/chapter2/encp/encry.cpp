#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>


#define BUF_SIZE 256

BOOL cci(LPCTSTR,LPCTSTR,DWORD);
VOID ReportError(LPTSTR lpMsg,DWORD val,BOOL showInfo);

int _tmain(int argc,LPTSTR argv[])
{

	if(argc != 4){
		printf(_T("Usage :encp  file1  file2  shift \n"));
		printf(_T("  shift may be +n or -n number \n"));
		return 0;
	}
      SYSTEMTIME st, lt;
      
     // GetSystemTime(&st);
	  GetLocalTime(&st);      
	  printf("The BEGIN time is: %02d:%02d:%02d - %04d\n", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);



	if(!cci(argv[1],argv[2],atoi(argv[3]))){
		ReportError("Encryping Error ",0,TRUE);
		return 1;
	} 

	GetLocalTime(&lt);
	printf("The  END  time is: %02d:%02d:%02d - %04d\n", lt.wHour, lt.wMinute, st.wSecond, st.wMilliseconds);
	
	return 0;
}

//exec 2-3 : enhance performance IMPROVEMENT larger than 4GB
/**FILE_FLAG_SEQUENCE_SCAN : 
Access is intended to be sequential from beginning to end. The system can use this as a hint to optimize file caching. 
This flag should not be used if read-behind (that is, backwards scans) will be used.
This flag has no effect if the file system does not support cached I/O and FILE_FLAG_NO_BUFFERING.
**/

/**
FILE_FLAG_NO_BUFFERING
The file or device is being opened with no system caching for data reads and writes. 
This flag does not affect hard disk caching or memory mapped files.
**/
/***
dwFlagsAndAttributes [in] 
The file or device attributes and flags, FILE_ATTRIBUTE_NORMAL being the most common default value for files. 
This parameter can include any combination of the available file attributes (FILE_ATTRIBUTE_*). 
All other file attributes override FILE_ATTRIBUTE_NORMAL.
This parameter can also contain combinations of flags (FILE_FLAG_*) for control of file or device caching behavior, access modes, 
and other special-purpose flags. These combine with any FILE_ATTRIBUTE_* values.
This parameter can also contain Security Quality of Service information by specifying the SECURITY_SQOS_PRESENT flag. 
Additional SQOS-related flags information is presented in the table following the attributes and flags tables.
Note  When CreateFile opens an existing file, it generally combines the file flags with the file attributes of the existing file, and ignores any file attributes supplied as part of dwFlagsAndAttributes. Special cases are detailed in Creating and Opening Files. 
Some of the following file attributes and flags may only apply to files and not necessarily all other types of devices that CreateFile can open. For additional information, see the Remarks section of this topic and Creating and Opening Files. 
**/

BOOL cci(LPCTSTR fIn,LPCTSTR fOut,DWORD shift){

	HANDLE f1 = CreateFile(fIn,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_FLAG_NO_BUFFERING,NULL);
	if(f1 == INVALID_HANDLE_VALUE){
		printf("Cannot open file %s ", fIn);
		return FALSE ;
	}

	DWORD nLen ,mLen;
	CHAR before[BUF_SIZE]  ; //,after[BUF_SIZE];
	BOOL isOK = true;
 

	HANDLE f2 = CreateFile(fOut,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_FLAG_NO_BUFFERING,NULL);
	if(f2 == INVALID_HANDLE_VALUE){
		printf("Cannot write to file\n");
		return FALSE;
	}

	while(ReadFile(f1,before,BUF_SIZE,&nLen,NULL) && (nLen >0) && isOK){
		for(int icHAR = 0 ; icHAR < nLen; icHAR++){
			before[icHAR] = (before[icHAR] + shift) % 256; 
		}
		isOK = WriteFile(f2,before,nLen,&mLen,NULL);
	}  
	CloseHandle(f1);
	CloseHandle(f2);

	return isOK;
} 