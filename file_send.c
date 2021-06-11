#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE        1024    // buf size
void error_handling(char *message);

// msg header define
typedef struct {
        int id;
        char name[30];
        int cmd_code;
}HEAD;
typedef struct {
        int cmd_code;
        char file_name[1024];
}res;

// integer to ascii
void itoa(int i, char *st)
{
        sprintf(st, "%d", i);
        return;
}

// string test function
void str_test()
{
        int i=3500;
        char id[5]={0, };       
        char age[4]="19";

        // integer --> char
        itoa(i, id);
        printf("itoa(%d) --> %s\n",i, id);

        // char --> integer
        i = atoi(age);
        printf("atoi(%s) --> %d\n",age, i);

}

int main()
{       
        int sock;
        struct sockaddr_in serv_addr;
        char buf[1024];
        int str_len;
        int port = 9100;
        char ip[20]="192.168.100.3";


        HEAD sheader, rheader;
        char msg[BUF_SIZE], rmsg[BUF_SIZE];
        // temporary variable
        char id[5]={0, }, age[4]={0, };
        char cmd_code[5] = {0,};
        int off;        // offset

        FILE *fp = NULL;
        FILE *fp2 = NULL;
        int nbyte=0;

        res res1;
        
        sock=socket(PF_INET, SOCK_STREAM, 0);
        if(sock == -1)
                error_handling("socket() error");
        
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family=AF_INET;
        serv_addr.sin_addr.s_addr=inet_addr(ip);
        serv_addr.sin_port=htons(port);
                
        if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1) 
                error_handling("connect() error!");
        
        str_len=read(sock, buf, sizeof(buf)-1);
        if(str_len==-1)
                error_handling("read() error!");
        
        printf("message from server: %s \n", buf);  

        // Sender  input data
        memset(&sheader, 0, sizeof(HEAD));
        sheader.id = 3623;
        strcpy(sheader.name, "youngsouk");
        sheader.cmd_code = 1030;

        //id
        memset(&msg, 0, BUF_SIZE);
        itoa(sheader.id, id);
        memcpy(&msg, id, sizeof(int));
        off = sizeof(int);

        //name
        memcpy(&msg[off], &sheader.name, sizeof(sheader.name));
        off += sizeof(sheader.name);

        //cmd_code
        itoa(sheader.cmd_code, cmd_code);
        memcpy(&msg[off], cmd_code, sizeof(int));
        off += sizeof(int);

        write(sock, msg, BUF_SIZE);
        printf("SEND MSG : %s\n", msg);

        memset(buf, 0, BUF_SIZE);
        str_len = read(sock, buf, BUF_SIZE);
        if(str_len==-1)
                error_handling("read() error!");
        printf("\n%s\n", buf);


        off=0;
        //cmd_code
        memset(&res1, 0, sizeof(res));
        
        memcpy(&id, buf, sizeof(int));

        res1.cmd_code = atoi(id);
        off = sizeof(int);
        printf("%d", res1.cmd_code);


        //file_name
        memcpy(&msg[off], buf, sizeof(res1.file_name));
        off += sizeof(res1.file_name);
        

}
void error_handling(char *buf)
{
        fputs(buf, stderr);
        fputc('\n', stderr);
        exit(1);
}

