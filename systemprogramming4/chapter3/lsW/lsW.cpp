#include <Windows.h>
#include <string.h>
#include <tchar.h>
#include <stdarg.h>

//C �⺯�� void *memchr(const void *str, int c, size_t n) 
//�ڲ��� str ��ָ����ַ�����ǰ n ���ֽ���������һ�γ����ַ� c��һ���޷����ַ�����λ�á�
#ifdef _UNICODE
  #define _memtchr wmemchr
#else
  #define _memtchr memchr
#endif

#define MAX_OPTIONS 3
 

/**
#define TYPE_FILE  0
#define TYPE_DOT	1
#define TYPE_DIR   2**/
#define TYPE_FILE 1    /* Used in ls, rm, and lsFP */
#define TYPE_DIR 2
#define TYPE_DOT 3

 
BOOL TraverseDirectory(LPCTSTR,DWORD,LPBOOL);
DWORD FileType(LPWIN32_FIND_DATA);
BOOL ProcessItem(LPWIN32_FIND_DATA,DWORD,LPBOOL);

//evaluate command line option flag and return the argv index of the first file name.
//use it in much the same way as getopt is used in many UNIX programs.
//getopt����������������ѡ������ģ�����ֻ�ܽ�����ѡ��: -d 100,���ܽ�����ѡ�--prefix
/**
argc��main()�������ݹ����Ĳ����ĸ���
argv��main()�������ݹ����Ĳ������ַ���ָ������
optstring��ѡ���ַ�������֪ getopt()���Դ����ĸ�ѡ���Լ��ĸ�ѡ����Ҫ����

**/
DWORD Options (int argc, LPCTSTR argv [], LPCTSTR OptStr, ...)

/*  
argv��������
ѡ�����еĻ�����argv [1]��argv [2]��...�е�'-'��ͷ
OptStr�ǰ������п���ѡ����ı��ַ���������������б�...���в��������ĵ�ַһһ��Ӧ��
������argv [1]��argv [2]��...�г�����Ӧ��ѡ���ַ�ʱ����������Щ��־
����ֵ�ǳ��˿�ѡ��֮���, ��һ��������argv���� : Ŀ¼
	*/

{
	va_list pFlagList;
	LPBOOL pFlag;
	int iOptions = 0, iArg;
	int _len = (int)_tcslen (OptStr);
	va_start (pFlagList, OptStr);

	while ((pFlag = va_arg (pFlagList, LPBOOL)) != NULL	&& iOptions < _len) {//���optionstr �е�ÿһ����ѡ
		*pFlag = FALSE;//�Ѿ��ҵ���1��ѡ��
		for (iArg = 1; !(*pFlag) && iArg < argc; iArg++){//��ѯ������
			char c = argv [iArg] [0];
			if(c != '-') continue;//����-��ͷ�Ŀ϶�����ѡ��, �������argv�ַ���
			int _slen = _tcslen (argv [iArg]);//���optionstr �еĵ�iOptions �������Ƿ�����
			*pFlag = _memtchr (argv [iArg], OptStr [iOptions],	_slen) != NULL;//LPBOOL is a  4 byte int
		}
		iOptions++;
	}

	va_end (pFlagList);
	_tprintf(_T("argv[1]=%s\n"),argv[1]);
	for (iArg = 1; iArg < argc && argv [iArg] [0] == '-'; iArg++);

	return iArg;
}


/* Chapter 3. Windows lsW file list command. */
/* lsW [options] [files]
	List the attributes of one or more files.
	Options:
		-R	Recursive
		-l	Long listing (size and time of modification)
			Depending on the ProcessItem function, this will
			also list the owner and permissions (See Chapter 5 - security). */

/* This program illustrates:
		1.	Search handles and directory traversal
		2.	File attributes, including time
		3.	Using generic string functions to output file information */


int _tmain(int argc ,LPTSTR argv[])
{
	BOOL flags[MAX_OPTIONS],ok = TRUE;
	TCHAR pathName[MAX_PATH + 1],currPath[MAX_PATH + 1],tempPath[MAX_PATH + 1];
	LPTSTR pSlash, pFileName;
	int i , fileIndex;

//	fileIndex = Options(argc,(LPCTSTR*)argv,_T("R1"),&flags[0],&flags[1],NULL);
	//���r-l-t ˳�����Ҫ ,��������ǰ���ֵ��flags���ݵ�.
	fileIndex = Options (argc, (LPCTSTR*)argv, _T("rl"), &flags [0], &flags [1] , NULL);

	GetCurrentDirectory(MAX_PATH,currPath);
	if(argc < fileIndex  +1 )
		ok = TraverseDirectory(_T("*"),MAX_OPTIONS , flags);
	else for(i = fileIndex; i < argc; i++){
		_tcscpy_s(pathName,argv[i]);
		_tcscpy_s(tempPath,argv[i]);

		//pSlash = _tstrrchr(tempPath,'\\');
		pSlash = _tcsrchr(tempPath,'\\');
		if(pSlash != NULL){
			*pSlash = '\0';
			_tcscat_s(tempPath,_T("\\"));
			SetCurrentDirectory(tempPath);
			//pSlash = _tstrrchr(pathName,'\\');
			pSlash = _tcsrchr(pathName,'\\');
			pFileName = pSlash + 1;
		}else{
			 pFileName = pathName;
		}
		ok = TraverseDirectory(pFileName,MAX_OPTIONS, flags) && ok;
		SetCurrentDirectory(currPath);
	}
}

//����Ŀ¼,�õķ�������ʡ��ע��
static BOOL TraverseDirectory(LPCTSTR pathName, DWORD numFlags,LPBOOL flags)
{
	HANDLE searchHandle;
	WIN32_FIND_DATA findData;
	BOOL recursive = flags[0];//�����1������r
	DWORD fType, iPass;
	TCHAR currPath[MAX_PATH + 1];
	GetCurrentDirectory(MAX_PATH, currPath);
	//��һ�������ļ����ڶ�������Ŀ¼��
	for(iPass = 1; iPass <= 2; iPass++){
		searchHandle = FindFirstFile(pathName,&findData);
		do{
			fType = FileType(&findData);
			if(iPass == 1)
				ProcessItem(&findData,MAX_OPTIONS,flags);//�����2������ l 
			if(fType == TYPE_DIR && iPass == 2 && recursive){
				_tprintf(_T("\n%s\\%s:"),currPath,findData.cFileName);
				SetCurrentDirectory(findData.cFileName);
				TraverseDirectory(_T("*"),numFlags,flags);
				SetCurrentDirectory(_T(".."));
			}
		}while(FindNextFile(searchHandle,&findData));//Continues a file search from a previous call to the FindFirstFile, FindFirstFileEx, or FindFirstFileTransacted functions.
		FindClose(searchHandle);
	}

	return TRUE;
}

static BOOL ProcessItem(LPWIN32_FIND_DATA pFileData,DWORD numFlags,LPBOOL flags)
{
	const TCHAR fileTypeChar[] = {' ','d'};
	DWORD fType = FileType(pFileData);
	BOOL isCmdLineSet = flags[1];//д����,�ڶ��������Ƿ�����
	SYSTEMTIME  lastWrite;

	if(fType != TYPE_FILE && fType != TYPE_DIR) return FALSE;
	_tprintf(_T("\n"));
	if(isCmdLineSet){
		_tprintf(_T("%c"),fileTypeChar[fType -1 ]);
		_tprintf(_T("%10d"),pFileData->nFileSizeLow);
		FileTimeToSystemTime(&(pFileData->ftLastWriteTime),&lastWrite);
		_tprintf(_T(" %02d/%02d/%04d %2d:%02d:%02d"),
			lastWrite.wMonth,lastWrite.wDay,
			lastWrite.wYear,lastWrite.wHour,
			lastWrite.wMinute,lastWrite.wSecond);
	
	}  
	_tprintf(_T(" %s"),pFileData->cFileName);
	return TRUE;
}

/* Return file type from the find data structure.
	Types supported:
		TYPE_FILE:	If this is a file
		TYPE_DIR:	If this is a directory other than . or ..
		TYPE_DOT:	If this is . or .. directory */
static DWORD FileType(LPWIN32_FIND_DATA pFileData)
{
	BOOL isDir;
	DWORD fType;
	fType = TYPE_FILE;
	isDir = (pFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

	if(isDir){
		if(lstrcmp(pFileData->cFileName, _T(".")) == 0 || lstrcmp(pFileData->cFileName,_T("..")) == 0){
			fType = TYPE_DOT;
		}else {
			fType = TYPE_DIR;
		}
	}
	return fType;
}