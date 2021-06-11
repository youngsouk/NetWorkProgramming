#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>

#define TTL	2
#define msg_SIZE 1024

typedef struct _client_t{
	char code[5];
	char ip[20];
	// char password[10];
	char chat; // '0' : off, '1' : online , '2': chatting  - 관리하기 쉽게 한자리 문자로 관리 
}client_t;
void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

int main(int argc, char *argv[]){
	int recv_sock, send_sock; //send : multicast
	int time_live = TTL;
	char ip[msg_SIZE];
	struct sockaddr_in radr, clientaddr; // udp recv struct
	struct sockaddr_in adr, sadr; //multicast 
	struct sockaddr r_adr;
	struct ip_mreq join_adr;
	int on=1, state, maxfd = 2;
	struct timeval tv; 
	char buf[msg_SIZE];
	int r_adr_sz;

	char code_cmp[5], i;

	client_t client_list[10]; // client list
	int delay[10] = {0,}; // client들의 delay 정보
	int client_cnt = 0;
	memset(client_list, 0, sizeof(client_list));

	fd_set readfds, tmp;
	
	if(argc!=3) {
		printf("Usage : %s <selfIP> <PORT>\n", argv[0]);
		exit(1);
	 }
	 // IP broadcating
	 //send
	
	send_sock = socket(PF_INET, SOCK_DGRAM, 0);
 	memset(&adr, 0, sizeof(adr));
	sadr.sin_family=AF_INET;
	sadr.sin_addr.s_addr=inet_addr("239.0.110.1");
	sadr.sin_port=htons(9000);

	on=0;
//	setsockopt(send_sock, IPPROTO_IP, IP_MULTICAST_LOOP, &on, sizeof(on));
	setsockopt(send_sock, IPPROTO_IP, 
		IP_MULTICAST_TTL, (void*)&time_live, sizeof(time_live));

	sprintf(ip, "%s %s", argv[1], argv[2]);
	// printf("%s\n", ip);

	 //udp recv sock
	recv_sock = socket(PF_INET, SOCK_DGRAM, 0);   
	if(recv_sock==-1)
		error_handling("socket() error\n");
	memset(&radr, 0, sizeof(radr));
	radr.sin_family=AF_INET;
	radr.sin_addr.s_addr=htonl(INADDR_ANY);
	radr.sin_port=htons(atoi(argv[2]));
	// printf("%d", atoi(argv[2]));

	if(bind(recv_sock, (struct sockaddr*)&radr, sizeof(radr))==-1)
		error_handling("bind() error\n");
	// 

	tv.tv_sec = 3;
	tv.tv_usec = 0;

	FD_ZERO(&readfds);
	FD_SET(recv_sock, &readfds);

	
	maxfd = recv_sock;
	tmp = readfds;
	 for(;;){
	 	state = select(maxfd+1, &readfds, (fd_set *) 0,  (fd_set *)0, &tv);
	 	switch(state){
	 		case -1:
                perror("select error : \n");
                exit(0);
                break;    
            case 0:
            	sendto(send_sock, ip, msg_SIZE, 0,(struct sockaddr *)&sadr, sizeof(sadr));
            	// printf("multicast : %s\n", ip);


            	if(client_cnt > 0){
            		printf("-------------------------------------------\n");
	            	for(int i = 0; i < 10; i++){
	            		if(client_list[i].code[0] != 0){
	            			printf("[%d] 학번 : %s ip : %s status : %c \n", i, client_list[i].code, client_list[i].ip,client_list[i].chat);
	            			delay[i]++; // delay 값 상승 4까지 가면 오프
	            		}
	            	}
	            	printf("-------------------------------------------\n");
            	}
            	
	            tv.tv_sec = 3;
				tv.tv_usec = 0;

				break;
	 		default:
	 			r_adr_sz = sizeof(r_adr);
	 			memset(buf, 0, sizeof(buf));

	 			if(FD_ISSET(recv_sock, &readfds)){
	 				memset(code_cmp, 0, sizeof(code_cmp));
	 				recvfrom(recv_sock, buf, msg_SIZE - 1, 0, (struct sockaddr *)&r_adr, &r_adr_sz);
	 				strncpy(code_cmp, buf, 4);

	 				// printf("학번 : %s\n", code_cmp);

	 				for(i = 0; i < 10; i++){
	 					if(strncmp(client_list[i].code, code_cmp, 4) == 0){
	 						strncpy(&client_list[i].chat, buf + 25, 1); // client chat 상태 업데이트
	 						delay[i] = 0; //client delay 업데이트
	 						break;
	 					}
	 				}
	 				if(i == 10){
	 					for(i = 0; i< 10; i++){
	 						if(client_list[i].code[0] == 0){
	 							client_cnt++;
	 							strncpy(client_list[i].code, buf, 4); // 학번 입력 
	 							strncpy(client_list[i].ip, buf + 5, 11);// IP 입력
			 					strncpy(&client_list[i].chat, buf + 25, 1); // 상 입력
			 					break;
	 						}
	 					}
	 					
	 				}
	 				for(i = 0; i < 10; i++){
	 					if(delay[i] >= 4){ // delay 4 즉 12초동안 heartbeat 오지 않음
	 						client_list[i].chat = '0';	// 오프라인 설정
	 					}
	 				}

	 			}
	 			break;
	 			
	 	}
	 	readfds = tmp;
	 }
	
	


	close(send_sock);
	return 0;
}