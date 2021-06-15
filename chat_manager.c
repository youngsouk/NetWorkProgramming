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

typedef struct _uinfo{
	char code[5];
	char id[20];
	char pw[20];
	char nickname[20];
}u_info;

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

FILE * get_user_info(u_info * u_info_p, char * file_name){
	char buf[1024];
	char * tmp;
	FILE * fd = fopen(file_name , "r+");

	memset(buf, 0, sizeof(buf));
	if(0 == fd){
		perror("file open error\n");
		exit(-1);
	}

	for(int i = 0; i < 10; i++){
		tmp = fgets(buf, sizeof(buf), fd);
		if(tmp == 0) break;
		tmp = strtok(buf, " ");
		strncpy(u_info_p[i].code, tmp, 4);
		tmp = strtok(0, " ");
		strncpy(u_info_p[i].id, tmp, strlen(tmp));
		tmp = strtok(0, " ");
		strncpy(u_info_p[i].pw, tmp, strlen(tmp));
		tmp = strtok(0, " ");
		strncpy(u_info_p[i].nickname, tmp, strlen(tmp));
	}
	// printf("%s %s %s \n", u_info_p[0].code, u_info_p[0].id, u_info_p[0].pw);
	// getchar();

}

int main(int argc, char *argv[]){
	int recv_sock, send_sock; //send : multicast
	int info_cmd_sock; // client info 요청
	int login_sock; // client login 요청

	int time_live = TTL;
	char ip[msg_SIZE];

	struct sockaddr_in radr, clientaddr; // udp recv struct
	struct sockaddr_in info_cmd_addr; // udp info commandrecv struct
	struct sockaddr_in login_addr; // udp login recv struct
	struct sockaddr_in adr, sadr; //multicast 
	struct sockaddr r_adr; // udp heart beat recv
	struct sockaddr r_adr2; // udp info cmd recv
	struct sockaddr r_adr3; // udp login cmd recv
	struct ip_mreq join_adr;

	int on=1, state, maxfd = 2;
	struct timeval tv; 
	char buf[msg_SIZE];
	int r_adr_sz;

	char code_cmp[5], i;

	client_t client_list[10]; // client list
	u_info u_info_list[10]; //uinfo list
	int login;
	FILE * fp;
	int delay[10] = {0,}; // client들의 delay 정보
	int client_cnt = 0;
	int str_len;
	char * string_tmp;
	memset(u_info_list, 0, sizeof(u_info_list));
	memset(client_list, 0, sizeof(client_list));

	fd_set readfds, tmp;
	
	if(argc!=3) {
		printf("Usage : %s <selfIP> <PORT>\n", argv[0]);
		exit(1);
	 }
	 // IP broadcating
	 //send
	fp = get_user_info(u_info_list, "UserInfo");
	
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
		error_handling("heart_bear bind() error\n");
	// 
	//udp cmd recv sock
	info_cmd_sock = socket(PF_INET, SOCK_DGRAM, 0);   
	if(info_cmd_sock==-1)
		error_handling("socket() error\n");

	memset(&info_cmd_addr, 0, sizeof(info_cmd_addr));
	info_cmd_addr.sin_family=AF_INET;
	info_cmd_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	info_cmd_addr.sin_port=htons(atoi(argv[2]) + 1);
	// printf("%d", atoi(argv[2]));

	if(bind(info_cmd_sock, (struct sockaddr*)&info_cmd_addr, sizeof(info_cmd_addr))==-1)
		error_handling("binfo ind() error\n");

	//udp login recv sock
	login_sock = socket(PF_INET, SOCK_DGRAM, 0);   
	if(login_sock==-1)
		error_handling("socket() error\n");

	memset(&login_addr, 0, sizeof(login_addr));
	login_addr.sin_family=AF_INET;
	login_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	login_addr.sin_port=htons(atoi(argv[2]) + 2);
	// printf("%d", atoi(argv[2]));

	if(bind(login_sock, (struct sockaddr*)&login_addr, sizeof(login_addr))==-1)
		error_handling("login bind() error\n");


	tv.tv_sec = 3;
	tv.tv_usec = 0;

	FD_ZERO(&readfds);
	FD_SET(recv_sock, &readfds);
	FD_SET(info_cmd_sock, &readfds);
	FD_SET(login_sock, &readfds);
	
	maxfd = login_sock;
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
	            		memset(buf, 0, sizeof(buf));
	            		if(client_list[i].code[0] != 0){
	            			if(client_list[i].chat == '0') sprintf(buf, "[%d] 학번 : %s ip : %s status : %s \n", i, client_list[i].code, client_list[i].ip, "offline");
	            			else if(client_list[i].chat == '1') sprintf(buf, "[%d] 학번 : %s ip : %s status : %s \n", i, client_list[i].code, client_list[i].ip, "online");
	            			else if(client_list[i].chat == '2') sprintf(buf, "[%d] 학번 : %s ip : %s status : %s \n", i, client_list[i].code, client_list[i].ip, "chatting");
	            			printf("%s",buf);
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
	 							strncpy(client_list[i].ip, buf + 5, 12);// IP 입력
			 					strncpy(&client_list[i].chat, buf + 25, 1); // 상태 입력
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
	 			if(FD_ISSET(info_cmd_sock, &readfds)){
	 				memset(buf, 0, sizeof(buf));
	 				recvfrom(info_cmd_sock, buf, msg_SIZE -1, 0, (struct sockaddr *)&r_adr2, &r_adr_sz);
	 				// printf("%s\n", buf);
	 				if(strncmp(buf, "INFO", 4) == 0) str_len = sendto(info_cmd_sock, (char *) client_list, sizeof(client_list), 0, (struct sockaddr *)&r_adr2, r_adr_sz);
	 				// printf("send %d byte\n", str_len);

	 			}
	 			if(FD_ISSET(login_sock, &readfds)){
	 				login = 0;
	 				// printf("로그인 요청 감지 \n");
	 				memset(buf, 0, sizeof(buf));
	 				recvfrom(login_sock, buf, msg_SIZE -1, 0, (struct sockaddr *)&r_adr3, &r_adr_sz);
	 				// printf("%s\n", buf);


	 				for(int i = 0; i < 10; i++){
	 					string_tmp = strtok(buf, " ");
	 					if(strcmp(u_info_list[i].code, buf) == 0){
	 						string_tmp = strtok(0, " ");
	 						// printf("id : %s ", string_tmp);
	 						if(strcmp(u_info_list[i].id, string_tmp) == 0){
	 							string_tmp = strtok(0, " ");
	 							// printf("pw : %s", string_tmp);
	 							if(strcmp(u_info_list[i].pw, string_tmp) == 0){
	 								memset(buf, 0, sizeof(buf));
	 								sprintf(buf, "success %s", u_info_list[i].nickname);
	 								printf("%s님이 로그인 하셨습니다. \n", u_info_list[i].nickname);
		 							sendto(login_sock, buf, strlen(buf), 0, (struct sockaddr *)&r_adr3, r_adr_sz);
		 							login = 1;
		 							break;
		 						}
	 						}
	 					}
	 				}
	 				if(login == 0){
	 					sendto(login_sock, "fail", 4, 0, (struct sockaddr *)&r_adr3, r_adr_sz);
		 				break;
	 				}

	 			}
	 			break;
	 			
	 	}
	 	readfds = tmp;
	 }
	
	close(send_sock);
	return 0;
}