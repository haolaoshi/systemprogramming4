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