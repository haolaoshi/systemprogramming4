#include <iostream>
#include <stdio.h>
#define BUF_SIZE	256

int main(int argc, char** argv){
	if(argc != 3){
		printf("Pls input two params");
		return 1;
	}

	size_t  nSize;



	char buffer[BUF_SIZE];
	FILE*  in = fopen(argv[1],"rb");
	if(in == NULL){
		printf("error open file1");
		return 2;
	}
	FILE* out = fopen(argv[2],"wb");
	if(out == NULL){
		printf("error open file");
		return 3;
	}


	while(nSize = fread(buffer,1,BUF_SIZE,in)> 0 ){

		fwrite(buffer,1,nSize,out);

	}
	fclose(in);
	fclose(out);

	system("pause");
	return 0;
}