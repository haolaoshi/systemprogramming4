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


VOID ReportError(LPTSTR lpMsg,DWORD val,BOOL showInfo){

	DWORD i  = GetLastError();
	LPTSTR buffer;

	_ftprintf(stderr,_T("lpMsg = %s"),lpMsg);//Print formatted data to a stream

	if(showInfo){

		DWORD msgLen =  FormatMessage(
		  FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,//   dwFlags,
		  NULL,// lpSource,
		  i,//   dwMessageId,
		  NULL, //  dwLanguageId,
		  (LPTSTR)&buffer,//  lpBuffer,
		  0,//   nSize,
		  NULL// *Arguments
		);

		if(msgLen > 0){
			_tprintf(_T("%s : %s\n"),lpMsg,buffer);
		}else{
			_tprintf(_T("Fatal Error when format %s : %d "),lpMsg,i);
		}
	}

	if(val > 0 ){
		ExitProcess(val);
	}
}
int _tmain (int argc, LPTSTR argv [])
{
	BOOL Flags[2], ok = TRUE;
	TCHAR KeyName [MAX_PATH+1];
	LPTSTR pScan;
	DWORD i, KeyIndex;
	HKEY hKey, hNextKey;

	/* Tables of predefined key names and keys */
	LPTSTR PreDefKeyNames [] = {
		_T("HKEY_LOCAL_MACHINE"),
		_T("HKEY_CLASSES_ROOT"),
		_T("HKEY_CURRENT_USER"),
		_T("HKEY_CURRENT_CONFIG"),
		NULL };
	HKEY PreDefKeys [] = {
		HKEY_LOCAL_MACHINE,
		HKEY_CLASSES_ROOT,
		HKEY_CURRENT_USER,
		HKEY_CURRENT_CONFIG };

		if (argc < 2) {
			_tprintf (_T("Usage: lsREG [options] SubKey\n"));
			return 1;
		}

	KeyIndex = Options (argc, (LPCTSTR*)argv, _T ("Rl"), &Flags[0], &Flags[1], NULL);

	/* "Parse" the search pattern into two parts: the "key"
		and the "subkey". The key is the first back-slash terminated
		string and must be one of HKEY_LOCAL_MACHINE, HKEY_CLASSES_ROOT,
		or HKEY_CURRENT_USER. The subkey is everything else.
		The key and subkey names will be copied from argv[KeyIndex]. */

	/*  Build the Key */
	pScan = argv[KeyIndex];
	for (i = 0; *pScan != _T('\\') && *pScan != _T('\0'); pScan++, i++)
		KeyName[i] = *pScan;
	KeyName[i] = _T('\0'); if (*pScan == _T('\\')) pScan++;

	/* Translate predefined key name to an HKEY */
	for (	i = 0;
			PreDefKeyNames[i] != NULL && _tcscmp (PreDefKeyNames[i], KeyName) != 0;
			i++) ;
	if (PreDefKeyNames[i] == NULL) ReportError (_T("Use a Predefined Key"), 1, FALSE);
	hKey = PreDefKeys[i];

	/*  pScan points to the start of the subkey string. It is not directly
		documented that \ is the separator, but it works fine */
	if (RegOpenKeyEx (hKey, pScan, 0, KEY_READ, &hNextKey) != ERROR_SUCCESS)
		ReportError (_T("Cannot open subkey properly"), 2, TRUE);
	hKey = hNextKey;

	ok = TraverseRegistry (hKey, argv[KeyIndex], NULL, Flags);

	return ok ? 0 : 1;
}

BOOL TraverseRegistry (HKEY hKey, LPTSTR FullKeyName, LPTSTR SubKey, LPBOOL Flags)

/*	Traverse a registry key, listing the name-value pairs and
	traversing subkeys if the -R option is set. 
	FullKeyName is a "full key name" starting with one of the open key
	names, such as "HKEY_LOCAL_MACHINE".
	SubKey, which can be null, is the rest of the path to be traversed. */

{
	HKEY hSubKey;
	BOOL Recursive = Flags [0];
	LONG Result;
	DWORD ValueType, Index;
	DWORD NumSubKeys, MaxSubKeyLen, NumValues, MaxValueNameLen, MaxValueLen;
	DWORD SubKeyNameLen, ValueNameLen, ValueLen;
	FILETIME LastWriteTime;
	LPTSTR SubKeyName, ValueName;
	LPBYTE Value;
	TCHAR FullSubKeyName [MAX_PATH+1];

	/* Open up the key handle. */

	if (RegOpenKeyEx (hKey, SubKey, 0, KEY_READ, &hSubKey) != ERROR_SUCCESS)
		ReportError (_T("\nCannot open subkey"), 2, TRUE);

	/*  Find max size info regarding the key and allocate storage */
	if (RegQueryInfoKey (hSubKey, NULL, NULL, NULL, 
		&NumSubKeys, &MaxSubKeyLen, NULL, 
		&NumValues, &MaxValueNameLen, &MaxValueLen, 
		NULL, &LastWriteTime) != ERROR_SUCCESS)
			ReportError (_T("Cannot query subkey information"), 3, TRUE);
	SubKeyName = (LPTSTR)malloc (MaxSubKeyLen+1);   /* size in characters w/o null */
	ValueName  = (LPTSTR)malloc (MaxValueNameLen+1);/* so add one to allow for null */
	Value      = (LPBYTE)malloc (MaxValueLen);      /* size in bytes */

	/*  First pass for name-value pairs. */
	for (	Index = 0; Index < NumValues; Index++) {
		ValueNameLen = MaxValueNameLen + 1; /* A very common bug is to forget to set */
		ValueLen     = MaxValueLen + 1;     /* these values, both function input and output */
		Result = RegEnumValue (hSubKey, Index, ValueName, &ValueNameLen, NULL,
				&ValueType, Value, &ValueLen);
		if (Result == ERROR_SUCCESS && GetLastError() == 0)
			DisplayPair (ValueName, ValueType, Value, ValueLen, Flags);
		/*  If you wanted to change a value, this would be the place to do it.
			RegSetValueEx (hSubKey, ValueName, 0, ValueType, pNewValue, NewValueSize); */
	}

	/*  Second pass for subkeys */
	for (Index = 0; Index < NumSubKeys; Index++) {
		SubKeyNameLen = MaxSubKeyLen + 1;
		Result = RegEnumKeyEx (hSubKey, Index, SubKeyName, &SubKeyNameLen, NULL,
				NULL, NULL, &LastWriteTime);
		if (GetLastError() == 0) {
			DisplaySubKey (FullKeyName, SubKeyName, &LastWriteTime, Flags);

			/*  Display subkey components if -R is specified */
			if (Recursive) {
				_stprintf (FullSubKeyName, _T("%s\\%s"), FullKeyName, SubKeyName);
				TraverseRegistry (hSubKey, FullSubKeyName, SubKeyName, Flags);
			}
		}
	}

	_tprintf (_T("\n"));
	free (SubKeyName); 
	free (ValueName);
	free (Value);
	RegCloseKey (hSubKey);
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

	case REG_EXPAND_SZ: /* 2: null-terminated string with unexpanded references to environment variables (for example, “%PATH%”). */ 
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