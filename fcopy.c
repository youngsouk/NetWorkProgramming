#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define	BUF_SIZE	100

int main()
{
	FILE *fp = NULL;
	FILE *fp2 = NULL;
	int nbyte=0;
	char buf[BUF_SIZE]={0, };

	// file open
	fp = fopen("./proto.c","rb");
	if ( fp == NULL )
	{
		printf(" fopen() error \n");
		exit(0);
	}

	fp2 = fopen("./p.txt","wb");
	if ( fp2 == NULL )
	{
		printf(" fopen() error \n");
		exit(0);
	}


	// Read data from proto.c :  BUF_SIZE
	while(1) 
	{
		memset(&buf, 0, BUF_SIZE);
		// file read
		nbyte = fread(buf, 1, BUF_SIZE, fp);
		if ( nbyte <= 0 ){
		       	break;
		}
		// new file write
		fwrite(buf, 1, nbyte, fp2);
	}

	fclose(fp);
	fclose(fp2);
}
