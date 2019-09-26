#include <stdio.h>

#define BUFSIZE   256

//standard C: ¡¡fread fwrite fclose fopen
int main(int argc,char** argv)
{
	if(argc != 3){
		printf("Usage : cp  file1  file2 \n");
		printf("standard C: ¡¡fread fwrite fclose fopen \n");
		return 1;
	}

	FILE* f1 = fopen(argv[1],"rb");
	if(f1 == NULL){
		printf("open %s failed\n",argv[1]);
		return 2;
	}
	FILE* f2 = fopen(argv[2],"wb");
	if(f2 == NULL){
		printf("open %s  failed\n",argv[2]);
		return 3;
	}

	size_t  len;
	char buffer[BUFSIZE];
	while((len = fread(buffer,1,BUFSIZE,f1)) > 0){
		size_t out = fwrite(buffer,1,len,f2);
		if(out != len ) break;
	}
	fclose(f1);
	fclose(f2);
	printf("Done.");

	return 0;
}