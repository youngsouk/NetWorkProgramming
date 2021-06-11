#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define BUF_SIZE 1024

void error_handling(char *msg){
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}

typedef struct{
    int id;
    char name[30];
    int cmdCode;
} REQUEST_HEADER;

// integer to ascii
void itoa(int i, char *st){
    sprintf(st, "%d", i);
    return;
}

int main(){
    int sock;
    struct sockaddr_in serv_addr;
    char msg[BUF_SIZE];
    int str_len;
    char ip[20] = "192.168.100.3"; 
    int port = 9100;               
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);

    REQUEST_HEADER i_request;
    int off = 0;
    int id[5] = {0, };
    int cmdCode[5] = {0, };
    char i_request_msg[BUF_SIZE];
    i_request.id = 3623;                   
    strcpy(i_request.name, "youngsouk");
    i_request.cmdCode = 1030;
    memset(&i_request_msg, 0, BUF_SIZE);

    //id,name,cmdCode
    REQUEST_HEADER s_request;
    char s_request_msg[BUF_SIZE];
    s_request.id = i_request.id;
    strcpy(s_request.name, i_request.name);
    s_request.cmdCode = 2030; // CMD

    //id
    itoa(i_request.id, id);
    memcpy(&i_request_msg[off], &id, sizeof(i_request.id));
    off += sizeof(int);
    //name
    memcpy(&i_request_msg[off], &i_request.name, sizeof(i_request.name));
    off += sizeof(i_request.name);
    //cmd_code
    itoa(i_request.cmdCode, cmdCode);
    memcpy(&i_request_msg[off], &cmdCode, sizeof(i_request.cmdCode));
    off += sizeof(int);

    off = 0;

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error!");

    str_len = read(sock, msg, sizeof(msg));
    if (str_len == -1)
        error_handling("read() error!");

    printf("msg from server: %s \n", msg);

    itoa(s_request.id, id);
    memcpy(&s_request_msg[off], &id, sizeof(s_request.id));
    off += sizeof(int);
    memcpy(&s_request_msg[off], &s_request.name, sizeof(s_request.name));
    off += sizeof(s_request.name);
    itoa(s_request.cmdCode, cmdCode);
    memcpy(&s_request_msg[off], &cmdCode, sizeof(s_request.cmdCode));
    off += sizeof(int);

   
    printf("==> file info request: %d %s %d \n", i_request.id, i_request.name, i_request.cmdCode);
    write(sock, i_request_msg, sizeof(i_request_msg));

    memset(&msg, 0, BUF_SIZE);
    str_len = read(sock, msg, sizeof(msg));
    printf("<== file info response: %s %s \n\n", msg, &msg[4]);
    char file_name[30];
    strcpy(file_name, &msg[4]);

    printf("==> file request: %d %s %d \n", s_request.id, s_request.name, s_request.cmdCode);
    write(sock, s_request_msg, sizeof(s_request_msg));

    memset(&msg, 0, BUF_SIZE);
    str_len = read(sock, msg, sizeof(msg));
    printf("<== file response: %s %s\n", msg, &msg[4]);

    memset(&msg, 0, BUF_SIZE);
    char buf[BUF_SIZE] = {0, };
    FILE *fp = NULL;
    fp = fopen(file_name, "wb");
    while (1){
        memset(&buf, 0, BUF_SIZE);
        str_len = read(sock, buf, sizeof(buf));
        if (str_len <= 0)
            break;
        else fwrite(buf, 1, str_len, fp);
    }

    close(sock);
    return 0;
}