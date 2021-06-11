#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void error_handling(char *message);

int main()
{
	int sock;
	struct sockaddr_in serv_addr;
	char buf[1024];
	int str_len;
	int port = 5001;
	char ip[20]="10.0.2.4";
	char input[1024] = "c";
	
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

	while(strcmp(input, "q\n") != 0 && strcmp(input, "Q\n") != 0){
		memset(buf, 0, sizeof(buf));
		fgets(input, 1024, stdin);
		write(sock, input, sizeof(input));
		read(sock, buf, sizeof(buf));
		//strcat(buf, NULL);
		printf("message from server: %s \n", buf);
		
	}

	close(sock);
	return 0;
}

void error_handling(char *buf)
{
	fputs(buf, stderr);
	fputc('\n', stderr);
	exit(1);
}
