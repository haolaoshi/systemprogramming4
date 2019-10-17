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


int _tmain(int argc , LPTSTR argv[])
{
	FILETIME	newFileTime;
	LPFILETIME	pAccessFileTime =  NULL,pModifyTime = NULL;
	HANDLE pFile;
	BOOL setAccessTime, setModifyTime, NotCreateNew, maFlag;
	DWORD createFlag;
	int i, fileIndex;
	if(argc <= 1){
		_tprintf(_T("\tUsage: touch [-Options] filename \n\t-a\tsetAccessTime;\n\t-m\tsetModifyTime;\n\t-c\tNotCreateNew;"));
		return 0;

	}
	fileIndex = Options(argc,(LPCTSTR*)argv,_T("amc"),&setAccessTime,&setModifyTime,&NotCreateNew,NULL);
	maFlag = setAccessTime || setModifyTime;
	createFlag = NotCreateNew ? OPEN_EXISTING : OPEN_ALWAYS;

	for(i = fileIndex; i< argc ; i++){
		pFile = CreateFile(argv[i],GENERIC_READ | GENERIC_WRITE, 0,NULL, createFlag, FILE_ATTRIBUTE_NORMAL,NULL);
		GetSystemTimeAsFileTime(&newFileTime);
		if(setAccessTime || !maFlag) pAccessFileTime = &newFileTime;
		if(setModifyTime || !maFlag) pModifyTime = &newFileTime;
		SetFileTime(pFile,NULL,pAccessFileTime,pModifyTime);
		CloseHandle(pFile);
	}
	return 0;
}

/**
BOOL LockFileEx(
  HANDLE       hFile,
  DWORD        dwFlags,
  DWORD        dwReserved,
  DWORD        nNumberOfBytesToLockLow,
  DWORD        nNumberOfBytesToLockHigh,
  LPOVERLAPPED lpOverlapped
);
BOOL UnlockFileEx(
  HANDLE       hFile,
  DWORD        dwReserved,
  DWORD        nNumberOfBytesToUnlockLow,
  DWORD        nNumberOfBytesToUnlockHigh,
  LPOVERLAPPED lpOverlapped
);*/