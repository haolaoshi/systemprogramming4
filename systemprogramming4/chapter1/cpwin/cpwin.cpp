#include <Windows.h>
#include <stdio.h>

#define BUF_SIZE 256

int main(int argc , LPTSTR argv[])
{
	if(argc != 3){
		printf("Usage : cpwin file1 file2 \n");
		return 1;
	}

	while(!CopyFile(argv[1],argv[2],FALSE)){
		printf("Eorror fatal Copy File : %d ",GetLastError());
		return 2;
	}
	printf("Done.");

	return 0;
}