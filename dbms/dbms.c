#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <unistd.h>


#define DB_FILE_SIZE 2048
#define DB_SIZE 1024
#define NAME_SIZE 100
#define DATA_SIZE 100

struct element{
	int type;
	char * name;
	char * content;
}typedef element;

element * data[DB_SIZE];
int r_db_size = 0;

char * initialize(){
	FILE * fp;
	char * db = calloc(1, DB_FILE_SIZE);
	fp = fopen("./db","rb");
	if ( fp == NULL )
	{
		printf("fopen() error \n");
		exit(0);
	}

	for(int i = 0; i < DB_SIZE; i++){
		data[i] = (element * ) calloc(1, sizeof(element));

	}
	
	int nbyte = fread(db, 1, DB_FILE_SIZE, fp);
	if ( nbyte <= 0 ){
		printf("NO DB content\n");
		exit(0);
	}
	return db;
}

void * save_db(char * db){
	char *ptr;
	char *ptr2;
	int i;

	ptr = strtok(db, "\n"); 
	for(i = 0; i < DB_SIZE; i++){

		if(strncmp(ptr, "char", 4) == 0)
			data[i] -> type = 0;
		else if(strncmp(ptr, "int", 3) == 0)
			data[i] -> type = 0;
		else {
			printf("DB FILE error");
			exit(0);
		}
		data[i] -> name = (char * )calloc(1, NAME_SIZE);
		ptr = strstr(ptr, " ") + 1;
		ptr2 = strstr(ptr, " ");
		strncpy(data[i] -> name, ptr, ptr2-ptr);
		*((data[i] -> name) + strlen(data[i] -> name)) = 0;

		data[i] -> content = (char * )calloc(1, DATA_SIZE); 
		strcpy(data[i] -> content, ptr2 + 1);

		printf("type : %d\n", data[i] -> type);
		printf("name : %s\n", data[i] -> name);
		printf("content : %s\n", data[i] -> content);
		ptr = strtok(0, "\n"); 
		if(ptr == 0) break;
	}
	r_db_size = i;
}

void interact(){
	int shmid, shmid2; 
    int pid; 
 
    int *cal_num; 
    void *shared_memory = (void *)0; 
    void *shared_memory2 = (void *)0; 
    char name[NAME_SIZE] = {0, };
    char nfind[NAME_SIZE] = "NOT FOUND";
    nfind[9] = 0;
 
 
    // 공유메모리 공간을 만든다. 
    shmid = shmget((key_t)1234, NAME_SIZE, 0666|IPC_CREAT); 
    shmid2 = shmget((key_t)1235, DATA_SIZE, 0666|IPC_CREAT); 
 
    if (shmid == -1) 
    { 
        perror("shmget failed : "); 
        exit(0); 
    } 
 
    // 공유메모리를 사용하기 위해 프로세스메모리에 붙인다.  
    shared_memory = shmat(shmid, (void *)0, 0); 
    shared_memory2 = shmat(shmid2, (void *)0, 0); 
    if (shared_memory == (void *)-1) 
    { 
        perror("shmat failed : "); 
        exit(0); 
    }

    memset(shared_memory, 0, DATA_SIZE);

    strcpy(shared_memory, nfind);
 	while(1){
 		if(strncmp((char*)shared_memory, nfind, strlen(nfind)) != 0){
 			strncpy(name, shared_memory, strlen(shared_memory));
 			strcpy(nfind, shared_memory);
 			for(int i = 0; i < r_db_size; i++){
 				if(strlen(name) == strlen(data[i]->name)){
	 				if(strncmp(data[i]->name, name, strlen(name)) == 0){
	 					strcpy(shared_memory2, data[i] -> content);
	 					printf("%s\n", data[i] -> content);
	 					break;
	 					memset(shared_memory, 0, strlen(name));
				 		memset(shared_memory2, 0, DATA_SIZE);
 					}	
 					else strcpy(shared_memory2, nfind);
 				}
 				else strcpy(shared_memory2, nfind);
 			}
 		}
 	}
}

int main(){
	char * db;
	printf("[+] Run DMBS system...\n");
	printf("initialize...\n");
	db = initialize();
	printf("initialize success\n");
	printf("db file parsing...\n");
	printf("0 : char 1: int\n");
	save_db(db);
	printf("db file parsing success\n");

	interact();

}	