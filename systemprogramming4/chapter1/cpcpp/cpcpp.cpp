#include <Windows.h>
#include <stdio.h>

#define BUF_SIZE 256
//copy file WINDOWS API version
//WINDOWS API : CreateFile  ReadFile WriteFile CloseHandle
int main(int argc, PTSTR argv[])
{
	if(argc != 3){
		printf("usage : cpcpp  f1  f2\n");
		return 1;
	}

	HANDLE hIn,hOut;
	TCHAR buffer[BUF_SIZE];
	DWORD inLen,outLen;

	hIn =  CreateFile(
	  argv[1],//                lpFileName,
	  GENERIC_READ,//                 dwDesiredAccess,
	  FILE_SHARE_READ,//                 dwShareMode,
	  NULL,// lpSecurityAttributes,
	  OPEN_EXISTING,//                 dwCreationDisposition,
	  FILE_ATTRIBUTE_NORMAL, //              dwFlagsAndAttributes,
	  NULL //                hTemplateFile
	);

	if(hIn == INVALID_HANDLE_VALUE){
		printf("Error open %s  :  %d \n",argv[1],GetLastError());
		return 1;
	}

	hOut = CreateFile(
		argv[2],
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);

	if(hOut == INVALID_HANDLE_VALUE){
		printf("Error oepn %s  : %d \n",argv[2],GetLastError());
		return 2;
	}

	while(ReadFile(hIn,buffer,BUF_SIZE,&inLen,NULL) && inLen > 0){
		WriteFile(hOut,buffer,inLen,&outLen,NULL);
		if(inLen != outLen) break;
	}

	CloseHandle(hIn);
	CloseHandle(hOut);
	printf("Down.");

	return 0;
}