/**
To determine the amount of available space correctly:
Use 64-bit math with the return values of both GetDiskFreeSpace()
and GetDiskFreeSpaceEx() so that you can determine the sizes of
volumes that are larger than 2GB.
Programs that need to determine how much free space the current user can have (such as whether
there is enough space to complete an installation)
have an additional requirement:
Use the lpFreeBytesAvailableToCaller value from
GetDiskFreeSpaceEx() rather than lpTotalNumberOfFreeBytes. 
This is because Windows 2000 has disk quota management that
administrators may use to limit the amount of disk space that users may use.
*/
#include <windows.h>
#include <stdio.h>
// typedef

typedef BOOL (WINAPI *P_GDFSE)(LPCTSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);



int wmain(int argc, WCHAR **argv)
{
	BOOL  fResult;//typedef int 4
	WCHAR  *pszDrive  = NULL, szDrive[4];//typedef wchar_t WCHAR;    // wc,   16-bit UNICODE character
	DWORD dwSectPerClust, dwBytesPerSect, dwFreeClusters, dwTotalClusters;//typedef unsigned long 4
	P_GDFSE pGetDiskFreeSpaceEx = NULL;//4
	unsigned __int64 i64FreeBytesToCaller, i64TotalBytes, i64FreeBytes;//8
wprintf(L"\tBOOL width:  %d  \n", sizeof(BOOL));
wprintf(L"\tWCHAR width:  %d  \n", sizeof(WCHAR));
wprintf(L"\tDWORD width:  %d  \n", sizeof(DWORD));
wprintf(L"\tP_GDFSE width:  %d  \n", sizeof(P_GDFSE));
wprintf(L"\tunsigned __int64 width:  %d  \n", sizeof(unsigned __int64));
	/* Command line parsing.
	If the drive is a drive letter and not a UNC path, append a trailing
	backslash to the drive letter and colon.  This is required on Windows 95 and 98. */

	if (argc != 2)
	{
		wprintf(L"usage:  %s <drive|UNC path>\n", argv[0]);
		wprintf(L"\texample:  %s C:\\\n", argv[0]);
		return 1;
	}

	pszDrive = argv[1];
	// Parse the drive
	if (pszDrive[1] == ':')
	{
		szDrive[0] = pszDrive[0];
		szDrive[1] = ':';
		szDrive[2] = '\\';
		szDrive[3] = '\0';
		pszDrive = szDrive;
	}



	/* Use GetDiskFreeSpaceEx() if available; otherwise, use GetDiskFreeSpace().
	Note: Since GetDiskFreeSpaceEx() is not in Windows 95 Retail, we
	dynamically link to it and only call it if it is present.  We
	don't need to call LoadLibrary() on KERNEL32.DLL because it is
	already loaded into every Win32 process's address space. */

	pGetDiskFreeSpaceEx = (P_GDFSE)GetProcAddress(GetModuleHandle (L"kernel32.dll"), "GetDiskFreeSpaceExW");
	if (pGetDiskFreeSpaceEx)
	{
		fResult = pGetDiskFreeSpaceEx((LPCTSTR)pszDrive, (PULARGE_INTEGER)&i64FreeBytesToCaller,
			(PULARGE_INTEGER)&i64TotalBytes, (PULARGE_INTEGER)&i64FreeBytes);
		if (fResult)
		{
			wprintf(L"\nGetDiskFreeSpaceExW reports:\n\n");
			wprintf(L"Available space to caller = %I64u MB\n", i64FreeBytesToCaller / (1024*1024));
			wprintf(L"Total space               = %I64u MB\n", i64TotalBytes / (1024*1024));
			wprintf(L"Free space on drive       = %I64u MB\n", i64FreeBytes / (1024*1024));
		}
	}
	else
	{
		fResult = GetDiskFreeSpace((LPCWSTR)pszDrive, &dwSectPerClust, &dwBytesPerSect, &dwFreeClusters, &dwTotalClusters);
		if (fResult)
		{
			/* force 64-bit math */
			i64TotalBytes = (__int64)dwTotalClusters * dwSectPerClust * dwBytesPerSect;

			i64FreeBytes = (__int64)dwFreeClusters * dwSectPerClust * dwBytesPerSect;
			wprintf(L"GetDiskFreeSpace reports\n");
			wprintf(L"Free space  = %I64u MB\n", i64FreeBytes / (1024*1024));
			wprintf(L"Total space = %I64u MB\n", i64TotalBytes / (1024*1024));
		}
	}
	if (!fResult)
		wprintf(L"error: %lu:  could not get free space for \%s\\n", GetLastError(), argv[1]);
	return 0;

}