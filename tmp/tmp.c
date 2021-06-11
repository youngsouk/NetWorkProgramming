#include <stdio.h>
#include <fcntl.h>

int main(void){
	int fd;
	char buf[]="Let's go!\n";
	fd = open("data.txt", O_CREAT | O_read
}     