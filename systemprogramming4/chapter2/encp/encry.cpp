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

	if(!cci(argv[1],argv[2],atoi(argv[3]))){
		ReportError("Encryping Error ",0,TRUE);
		return 1;
	} 

	return 0;
}

BOOL cci(LPCTSTR fIn,LPCTSTR fOut,DWORD shift){

	HANDLE f1 = CreateFile(fIn,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(f1 == INVALID_HANDLE_VALUE){
		printf("Cannot open file %s ", fIn);
		return FALSE ;
	}

	DWORD nLen ,mLen;
	CHAR before[BUF_SIZE]  ; //,after[BUF_SIZE];
	BOOL isOK = true;
 

	HANDLE f2 = CreateFile(fOut,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(f2 == INVALID_HANDLE_VALUE){
		printf("Cannot write to file\n");
		return FALSE;
	}

	while(ReadFile(f1,before,BUF_SIZE,&nLen,NULL) && (nLen >0) && isOK){
		for(int icHAR = 0 ; icHAR < nLen; icHAR++){
			before[icHAR] = (before[icHAR] + shift) % 256; 
		}
		printf(" ... %d ... \n",nLen);
		isOK = WriteFile(f2,before,nLen,&mLen,NULL);
	}  
	CloseHandle(f1);
	CloseHandle(f2);

	return isOK;
} 