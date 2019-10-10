#include <Windows.h>
#include <string.h>
#include <tchar.h>
#include <stdarg.h>

//C 库函数 void *memchr(const void *str, int c, size_t n) 
//在参数 str 所指向的字符串的前 n 个字节中搜索第一次出现字符 c（一个无符号字符）的位置。
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
//getopt是用来解析命令行选项参数的，但是只能解析短选项: -d 100,不能解析长选项：--prefix
/**
argc：main()函数传递过来的参数的个数
argv：main()函数传递过来的参数的字符串指针数组
optstring：选项字符串，告知 getopt()可以处理哪个选项以及哪个选项需要参数

**/
DWORD Options (int argc, LPCTSTR argv [], LPCTSTR OptStr, ...)

/*  
argv是命令行
选项（如果有的话）以argv [1]，argv [2]，...中的'-'开头
OptStr是包含所有可能选项的文本字符串，与变量参数列表（...）中布尔变量的地址一一对应。
仅当在argv [1]，argv [2]，...中出现相应的选项字符时，才设置这些标志
返回值是除了可选项之外的, 第一个参数的argv索引 : 目录
	*/

{
	va_list pFlagList;
	LPBOOL pFlag;
	int iOptions = 0, iArg;
	int _len = (int)_tcslen (OptStr);
	va_start (pFlagList, OptStr);

	while ((pFlag = va_arg (pFlagList, LPBOOL)) != NULL	&& iOptions < _len) {//检查optionstr 中的每一个可选
		*pFlag = FALSE;//已经找到第1个选项
		for (iArg = 1; !(*pFlag) && iArg < argc; iArg++){//轮询命令行
			char c = argv [iArg] [0];
			if(c != '-') continue;//不以-开头的肯定不是选项, 跳过这个argv字符串
			int _slen = _tcslen (argv [iArg]);//检查optionstr 中的第iOptions 个参数是否设置
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
	//这个r-l-t 顺序很重要 ,参数结果是按序赋值给flags数据的.
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

//遍历目录,好的方法名能省略注释
static BOOL TraverseDirectory(LPCTSTR pathName, DWORD numFlags,LPBOOL flags)
{
	HANDLE searchHandle;
	WIN32_FIND_DATA findData;
	BOOL recursive = flags[0];//处理第1个参数r
	DWORD fType, iPass;
	TCHAR currPath[MAX_PATH + 1];
	GetCurrentDirectory(MAX_PATH, currPath);
	//第一个处理文件，第二个处理目录。
	for(iPass = 1; iPass <= 2; iPass++){
		searchHandle = FindFirstFile(pathName,&findData);
		do{
			fType = FileType(&findData);
			if(iPass == 1)
				ProcessItem(&findData,MAX_OPTIONS,flags);//处理第2个参数 l 
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
	BOOL isCmdLineSet = flags[1];//写死了,第二个参数是否启用
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