#include <stdio.h>
#include <fcntl.h>

#include <stdlib.h>

size_t BUF_SIZE = 30;
int main(void) {
   int fd;
   char buf[BUF_SIZE];
      
   fd = open("data.txt", O_RDWR);
   if(fd==-1)   printf("open() error!");
   printf("file descriptor : %d \n", fd);
   
   if(read(fd, buf, sizeof(buf)) == -1)   printf("write() error!");
   printf("file data: %s", buf);
   close(fd);
   return 0;
}