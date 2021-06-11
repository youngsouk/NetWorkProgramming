#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <unistd.h>

#define DATA_SIZE 100

int main(){
	int shmid, shmid2;
	void *shared_memory = (void *)0; 
	void *shared_memory2 = (void *)0; 
	char buf[DATA_SIZE] = {0,};

	shmid = shmget((key_t)1234, DATA_SIZE, 0666|IPC_CREAT); 
	shmid2 = shmget((key_t)1235, DATA_SIZE, 0666|IPC_CREAT); 
	if (shmid == -1) 
    { 
        perror("shmget failed : "); 
        exit(0); 
    } 

    shared_memory = shmat(shmid, (void *)0, 0); 
    shared_memory2 = shmat(shmid2, (void *)0, 0); 
    if (shared_memory == (void *)-1) 
    { 
        perror("shmat failed : "); 
        exit(0); 
    }
    while(1){
        memset(buf, 0, sizeof(buf));
    	scanf("%s", buf);
    	buf[strlen(buf)] = 0;
	    strcpy(shared_memory, buf);
	    printf("%s\n", (char *) shared_memory2);
    }

	
}