#include "everything.h"




int _tmain(int argc, LPTSTR argv[])
{

	HANDLE hInfile,hStdin = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

	BOOL dash;
	int iArg,iFirstFile;

	iFirstFile = Options(argc,(LPCTSTR*)argv,_T("s"),&dash,NULL);

	if(iFirstFile == argc){
		CatFile(hStdin,hStdout);
		return 0;
	}

	//process each input files
	for(iArg = iFirstFile ; iArg < argc; iArg++){
		hInfile = CreateFile(
		  argv[iArg],	//LPCSTR                lpFileName,
		  GENERIC_READ,	//DWORD                 dwDesiredAccess,
		  0,			//DWORD                 dwShareMode,
		  NULL,			//LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		  OPEN_EXISTING,//DWORD                 dwCreationDisposition,
		  FILE_ATTRIBUTE_NORMAL,//dwFlagsAndAttributes,The file or device attributes and flags, FILE_ATTRIBUTE_NORMAL being the most common default value for files.
		  NULL			//HANDLE                hTemplateFile
		);
		if(hInfile == INVALID_HANDLE_VALUE){
			if(!dash) ReportError(_T("Open  File Error"),0,TRUE);
			return 1;
		}else{
			CatFile(hInfile,hStdout);
			if(GetLastError() != 0 && !dash){
				ReportError(_T("Error when Copy File"),0,TRUE);
			}
			CloseHandle(hInfile);
		}
	}
	return 0;
}

  // Open the existing file.
  /**
  CreateFile(TEXT("one.txt"), // open One.txt
	GENERIC_READ,             // open for reading
	0,                        // do not share
	NULL,                     // no security
	OPEN_EXISTING,            // existing file only
	FILE_ATTRIBUTE_NORMAL,    // normal file
	NULL); 

	**/