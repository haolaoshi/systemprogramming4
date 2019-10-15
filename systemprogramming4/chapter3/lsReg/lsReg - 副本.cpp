/* Chapter 3. lsREG - Registry list command. */
/* lsREG [options] SubKey 
	List the key-value pairs.
	Options:
		-R	Recursive
		-l  List extended information; namely, the last write time
			and the value type					*/

/* This program illustrates:
		1.	Registry handles and traversal
		2.	Registry values
		3.	The similarity and differences between directory
			and registry traversal 

	Note that there are no wild card file names and you specify the
	subkey, with all name-value pairs being listed. This is similar to 
	ls with "SubKey\*" as the file specifier             */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#include <Windows.h>
#include <tchar.h>

BOOL TraverseRegistry(HKEY,LPTSTR,LPTSTR,LPBOOL);
BOOL DisplayPair(LPTSTR,DWORD,LPBYTE,DWORD,LPBOOL);
BOOL DisplaySubKey(LPTSTR,LPTSTR,PFILETIME,LPBOOL);

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


int _tmain(int argc,LPTSTR argv[])
{
	BOOL flags[2],ok = TRUE;
	TCHAR keyName[MAX_PATH + 1];
	LPTSTR pScan;
	DWORD i,keyIndex;
	HKEY hKey,hNextKey;
	LPTSTR PreDefKeyNames[] = {
		_T("HKEY_LOCAL_MACHINE"),_T("HKEY_CLASSES_ROOT"),
		_T("HEKY_CURRENT_USER"),_T("HKEY_CURRENT_CONFIG"),NULL
	};

	HKEY PreDefKeys[] = {
		HKEY_LOCAL_MACHINE,HKEY_CLASSES_ROOT,
		HKEY_CURRENT_USER,HKEY_CURRENT_CONFIG
	};

	keyIndex = Options(
		argc,(LPCTSTR*)argv,_T("Rl"),&flags[0],&flags[1],NULL
		);
	pScan = argv[keyIndex];
	for(i = 0 ; pScan != _T("\\") && *pScan != _T('\0') && i < MAX_PATH; pScan++,i++)
		keyName[i] = *pScan;
	keyName[i] = _T('\0');
	if(*pScan == _T('\\')) pScan++;
	for(i = 0 ; PreDefKeyNames[i] != NULL && _tcscmp(PreDefKeyNames[i],keyName) != 0; i++);
	hKey = PreDefKeys[i];
	RegOpenKeyEx(hKey,pScan,0,KEY_READ,&hNextKey);
	hKey = hNextKey;

	ok = TraverseRegistry(hKey,argv[keyIndex],NULL,flags);
	RegCloseKey(hKey);
	return ok ? 0 : 1;
}

BOOL TraverseRegistry(HKEY hKey, LPTSTR fullKeyName, LPTSTR subKey, LPBOOL flags)
{

	HKEY hSubKey;
	BOOL recursive = flags[0];
	LONG result;
	DWORD valueType,index;
	DWORD numSubKeys , maxSubKeyLen, numValues,maxValueNameLen,maxValueLen;
	DWORD subKeyNameLen, valueNameLen, valueLen;
	FILETIME lastWriteTime;
	LPTSTR subKeyName,valueName;
	LPBYTE value;
	TCHAR fullSubKeyName[MAX_PATH + 1];
	RegOpenKeyEx(hKey , subKey, 0, KEY_READ, &hSubKey);

	RegQueryInfoKey(hSubKey,NULL, NULL, NULL, &numSubKeys, &maxSubKeyLen,NULL,&numValues,&maxValueNameLen,
		&maxValueLen,NULL,&lastWriteTime);
	subKeyName = (LPTSTR)malloc(maxSubKeyLen + 1);
	valueName = (LPTSTR)malloc( maxValueNameLen + 1);
	value     = (LPBYTE)malloc(maxValueLen);


	for(index = 0 ; index < numValues; index++){
		valueNameLen = maxValueNameLen + 1;
		valueLen = maxValueLen + 1;
		result = RegEnumValue(hSubKey,index,valueName,
			&valueNameLen, NULL,&valueType,value,&valueLen);

		if(result == ERROR_SUCCESS && GetLastError() == 0)
			DisplayPair(valueName, valueType,value,valueLen,flags);

	}
	for(index = 0; index < numSubKeys ; index++){
		subKeyNameLen = maxSubKeyLen +1;
		result = RegEnumKeyEx(hSubKey,index,subKeyName,
			&subKeyNameLen,NULL,NULL,NULL,&lastWriteTime);
		if(GetLastError() == 0){
			DisplaySubKey(fullKeyName,subKeyName,
				&lastWriteTime,flags);
			if(recursive){
				_stprintf (fullSubKeyName,_T("%s\\%s"),fullKeyName,
					subKeyName);
				TraverseRegistry(hSubKey,fullSubKeyName,subKeyName,flags);
			}
		}

	}
	_tprintf(_T("\n"));
	free(subKeyName);
	free(valueName);
	free(value);
	RegCloseKey(hSubKey);
	return TRUE;
}

BOOL DisplayPair (LPTSTR ValueName, DWORD ValueType,
						 LPBYTE Value, DWORD ValueLen,
						 LPBOOL Flags)

/* Function to display name-value pairs. */

{

	LPBYTE pV = Value;
	DWORD i;

	_tprintf (_T ("\nValue:  %s = "), ValueName);
	switch (ValueType) {
	case REG_FULL_RESOURCE_DESCRIPTOR: /* 9: Resource list in the hardware description */
	case REG_BINARY: /*  3: Binary data in any form. */ 
		for (i = 0; i < ValueLen; i++, pV++)
			_tprintf (_T(" %x"), *pV);
		break;

	case REG_DWORD: /* 4: A 32-bit number. */
		_tprintf (_T("%x"), (DWORD)*Value);
		break;

	case REG_EXPAND_SZ: /* 2: null-terminated string with unexpanded references to environment variables (for example, ��%PATH%��). */ 
	case REG_MULTI_SZ: /* 7: An array of null-terminated strings, terminated by two null characters. */
	case REG_SZ: /* 1: A null-terminated string. */ 
		_tprintf (_T("%s"), (LPTSTR)Value);
		break;
	
	case REG_DWORD_BIG_ENDIAN: /* 5:  A 32-bit number in big-endian format. */ 
	case REG_LINK: /* 6: A Unicode symbolic link. */
	case REG_NONE: /* 0: No defined value type. */
	case REG_RESOURCE_LIST: /* 8: A device-driver resource list. */
 	default: _tprintf (_T(" ** Cannot display value of type: %d. Exercise for reader\n"), ValueType);
		break;
	}

	return TRUE;
}

BOOL DisplaySubKey (LPTSTR KeyName, LPTSTR SubKeyName, PFILETIME pLastWrite, LPBOOL Flags)
{
	BOOL Long = Flags [1];
	SYSTEMTIME SysLastWrite;

	_tprintf (_T("\nSubkey: %s"), KeyName);
	if (_tcslen(SubKeyName) > 0) _tprintf (_T("\\%s "), SubKeyName);
	if (Long) {
		FileTimeToSystemTime (pLastWrite, &SysLastWrite);
		_tprintf (_T ("	%02d/%02d/%04d %02d:%02d:%02d"),
				SysLastWrite.wMonth, SysLastWrite.wDay,
				SysLastWrite.wYear, SysLastWrite.wHour,
				SysLastWrite.wMinute, SysLastWrite.wSecond);
	}
	return TRUE;

}