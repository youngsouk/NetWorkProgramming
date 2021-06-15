#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>

#define BUF_SIZE 1024
#define IP_STR_SIZE 100

typedef struct _client_t{
	char code[5];
	char ip[20];
	// char password[10];
	char chat; // '0' : off, '1': online , '2': chatting
}client_t;

typedef struct _uinfo{
	char code[5];
	char id[20];
	char pw[20];
	char nickname[20];
}u_info;

int main(int argc, char *argv[]){
	int state;
	int m_cast_rcv = socket(PF_INET, SOCK_DGRAM, 0);
	int hb_sock = socket(PF_INET, SOCK_DGRAM, 0);   
	int tcp_server_socket = socket(PF_INET, SOCK_STREAM, 0);
	int tcp_client_socket = socket(PF_INET, SOCK_STREAM, 0);
	int tcp_server_socket2 = -1; //accept()로 생성되는 소켓 저장
	int info_socket = socket(PF_INET, SOCK_DGRAM, 0); //INFO 명령어용
	int login_socket = socket(PF_INET, SOCK_DGRAM, 0); //LOGIN 명령어용

	struct sockaddr_in chat_server_addr; // chat tcp - server
	struct sockaddr_in chat_client_addr; // chat tcp - client
	struct sockaddr_in chat_server_addr2; // chat tcp - accept()로 받는 클라이언트 정보
	int first_connect = 0; //tcp 채팅 최초 연결
	int sock_struct_len = sizeof(chat_server_addr2); //accept()용 크기 저장 변수
	char is_server = -1; // -1 : not connnected 0 : client 1 : server 
	struct sockaddr_in m_addr; // multi
	struct ip_mreq join_addr; // multi
	char chat_m_ip[IP_STR_SIZE] = {0,}; // chat manager IP
	int chat_m_port = 0; // chat manager IP

	client_t client;
	client_t client_list[10]; // INFO 명령어용 배열
	char my_nick_name[20] = "unknown";
	char login = 0; // 0 : logout 1 : login

	struct sockaddr_in udp_serv_adr; //udp
	struct sockaddr_in udp_info_adr; //udp_info
	struct sockaddr_in udp_info_r_adr; //udp_info recv
	struct sockaddr_in udp_login_adr; //udp_login
	struct sockaddr_in udp_login_r_adr; //udp_login recv
	int udp_info_r_adr_sz = sizeof(udp_info_r_adr);

	int stdin_no = fileno(stdin); // stdin
	char u_input[BUF_SIZE]; // user input

	char * tmp; // sttok tmp
	char * tmp2; // sttok tmp

	int maxfd;
	fd_set readfds, read_tmp;
	struct timeval tv; 

	int str_len;
	char buf[BUF_SIZE];

	setvbuf(stdout, NULL, _IONBF, 0); 
	if(argc!=4) {
	    printf("Usage : %s <ip> <port> <학번>\n", argv[0]);
	    exit(1);
    }

    memset(&chat_server_addr, 0, sizeof(chat_server_addr)); //tcp_server_sock 구조체
    chat_server_addr.sin_family=AF_INET;
    chat_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    chat_server_addr.sin_port = htons(atoi(argv[2]));
    if(bind(tcp_server_socket, (struct sockaddr*) &chat_server_addr, sizeof(chat_server_addr)) == -1){
    	printf("bind() error\n");
		close(tcp_server_socket);
		exit(1);
    }
    listen(tcp_server_socket, 1);

	memset(&m_addr, 0, sizeof(m_addr)); // multicast 소켓 구조체
	m_addr.sin_family=AF_INET;
	m_addr.sin_addr.s_addr=htonl(INADDR_ANY);	
	m_addr.sin_port=htons(9000);
	
	int on=1;
	setsockopt(m_cast_rcv, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));


	if(bind(m_cast_rcv, (struct sockaddr*) &m_addr, sizeof(m_addr))==-1)
	{
		printf("bind() error\n");
		close(m_cast_rcv);
		exit(1);	
	}

	// Specify the multicast Group	
	join_addr.imr_multiaddr.s_addr=inet_addr("239.0.110.1");
	// Accept multicast from any interface
	join_addr.imr_interface.s_addr=htonl(INADDR_ANY);

	if ( (setsockopt(m_cast_rcv, IPPROTO_IP, 
		IP_ADD_MEMBERSHIP, (void*)&join_addr, sizeof(join_addr)))< 0 ) 
	{
		printf("SetsockOpt Join Error \n");
		close(m_cast_rcv);
		exit(1);
	}

	tv.tv_sec = 3;
	tv.tv_usec = 0;

	FD_ZERO(&readfds);

	// printf("%d\n", stdin_no);
	FD_SET(stdin_no, &readfds);
	FD_SET(m_cast_rcv, &readfds);
	FD_SET(tcp_server_socket, &readfds);

	read_tmp = readfds;
	maxfd = login_socket;

	memset(&client, 0, sizeof(client)); //clent 정보 셋팅
	strcpy(client.ip, argv[1]);
	strcpy(client.code, argv[3]);
	client.chat = '1';

	memset(&udp_serv_adr, 0, sizeof(udp_serv_adr));


	for(;;){
		state = select(maxfd+1, &readfds, (fd_set *) 0,  (fd_set *)0, &tv);
		switch(state){
			case -1:
				perror("select error : \n");
                exit(0);
                break;
            case 0:
				tv.tv_sec = 3;
				tv.tv_usec = 0;
				// printf("TIME OUT\n");
            	break;
            default:
            	memset(buf, 0, sizeof(buf));
            	if(FD_ISSET(m_cast_rcv, &readfds)){
            		str_len=recvfrom(m_cast_rcv, buf, BUF_SIZE, 0, NULL, 0);
            		tmp = strtok(buf, " "); 
            		strcpy(chat_m_ip, tmp); //ip 저장
            		tmp = strtok(0, " "); 
            		chat_m_port = atoi(tmp); // port 저장
            		// printf("IP : %s port : %d\n", chat_m_ip, chat_m_port);

            		udp_serv_adr.sin_family = AF_INET;
            		udp_serv_adr.sin_port = htons(chat_m_port);
            		udp_serv_adr.sin_addr.s_addr = inet_addr(chat_m_ip);

            		if(-1 == connect(hb_sock, (struct sockaddr*) &udp_serv_adr, sizeof(udp_serv_adr))){
            			printf("connect error\n");
            			close(hb_sock);
            			exit(1);
        			}


        			// printf("heart beat send\n");
            		str_len =sendto(hb_sock, (char *)&client, sizeof(client), 0, (struct sockaddr *)&udp_serv_adr, sizeof(udp_serv_adr));


            		if(str_len<0) 
						break;

            	}
            	if(FD_ISSET(tcp_server_socket, &readfds)){
            		printf("연결 요청 받음\n");
            		is_server =1;
            		tcp_server_socket2 = accept(tcp_server_socket, (struct sockaddr *)&chat_server_addr2, &sock_struct_len);
            		if(client.chat == '2'){

            			write(tcp_server_socket2, "이미 채팅중인 상대입니다.", 36);
            			close(tcp_server_socket2);
            			continue;
            		}
            		maxfd = tcp_server_socket2;
            		FD_SET(tcp_server_socket2, &read_tmp);
            		printf("연결 완료\n");
            		client.chat = '2';
            	}
            	if(FD_ISSET(tcp_server_socket2, &readfds)){
            		if(first_connect == 0){
            			first_connect++;
            			continue;
            		}
            		memset(buf, 0, sizeof(buf));
            		str_len = read(tcp_server_socket2, buf, sizeof(buf));
            		// printf("받은 문자열 길이 : %d\n", str_len);
            		if(str_len == -1 || str_len == 0){
            			printf("클라이언트와의 연결 끊김\n");
            			close(tcp_server_socket2);
            			is_server = -1;
            			client.chat = '1';
            			first_connect = 0;
            			FD_CLR(tcp_server_socket2, &read_tmp);
            			maxfd--;

            			continue;
            		}
            		// printf("채팅 연결 받음\n");
            		printf("%s", buf);
            	}
            	if(FD_ISSET(tcp_client_socket, &readfds)){

            		if(first_connect == 0){
            			first_connect++;
            			continue;
            		}
            		memset(buf, 0, sizeof(buf));
            		str_len = read(tcp_client_socket, buf, sizeof(buf));
            		// printf("%d\n", str_len);
            		if(str_len == -1 || str_len == 0){
            			printf("서버와의 연결 끊김\n");
            			close(tcp_client_socket);
        				tcp_client_socket = socket(PF_INET, SOCK_STREAM, 0);
        				FD_CLR(tcp_client_socket, &read_tmp);
        				is_server = -1;
            			client.chat = '1';
            			first_connect = 0;

            			readfds = read_tmp;
            			continue;
            		}
            		printf("%s", buf);
            	}
            	if(FD_ISSET(stdin_no, &readfds)){
            		memset(u_input, 0, sizeof(u_input));
            		fgets(u_input, BUF_SIZE, stdin);
            		tmp = strtok(u_input, " ");
            		if(strcmp(tmp, "CMD") == 0){
            			// printf("CMD MODE\n");
            			tmp = strtok(0, " ");
            			// printf("%d\n", strcmp(tmp, "INFO\n") == 0);
            			if(strcmp(tmp, "INFO\n") == 0){
            				memset(&udp_info_adr, 0, sizeof(udp_info_adr));
            				udp_info_adr.sin_family = AF_INET;
		            		udp_info_adr.sin_port = htons(chat_m_port + 1);
		            		udp_info_adr.sin_addr.s_addr = inet_addr(chat_m_ip);
		            		// printf("%s %d\n", chat_m_ip, chat_m_port + 1);

		            		if(-1 == connect(info_socket, (struct sockaddr*) &udp_info_adr, sizeof(udp_info_adr))){
		            			printf("UDP command server connect error\n");
		            			close(hb_sock);
		            			exit(1);
		        			}

		        			// printf("CONNECT success\n");
		        			memset(buf, 0, sizeof(buf));
		        			// printf("heart beat send\n");
		            		str_len = sendto(info_socket, "INFO", 4, 0, (struct sockaddr *)&udp_info_adr, udp_info_r_adr_sz);
		            		// printf("SEND info cmd success\n");
		            		str_len = recvfrom(info_socket,buf, BUF_SIZE -1, 0, (struct sockaddr *)&udp_info_r_adr, &udp_info_r_adr_sz);
		            		// printf("recv success \n");
		            		memcpy((char*)client_list, buf, sizeof(client_list));
		            		// printf("%d recv\n", str_len);

		            		// printf("recv client info success");

		            		printf("------------------------------------------------\n");

			            	for(int i = 0; i < 10; i++){
			            		memset(buf, 0, sizeof(buf));
			            		if(client_list[i].code[0] != 0){
			            			if(client_list[i].chat == '0') sprintf(buf, "[%d] 학번 : %s ip : %s status : %s \n", i, client_list[i].code, client_list[i].ip, "offline");
			            			else if(client_list[i].chat == '1') sprintf(buf, "[%d] 학번 : %s ip : %s status : %s \n", i, client_list[i].code, client_list[i].ip, "online");
			            			else if(client_list[i].chat == '2') sprintf(buf, "[%d] 학번 : %s ip : %s status : %s \n", i, client_list[i].code, client_list[i].ip, "chatting");
			            			printf("%s",buf);
			            		}
			            	}
			            	printf("------------------------------------------------\n");


            			}
            			if(strcmp(tmp, "QUIT\n") == 0){
            				printf("QUIT\n");
            				close(m_cast_rcv);
            				close(hb_sock);
            				close(tcp_server_socket);
            				close(tcp_client_socket);

            				exit(0);
            			}
            		}
            		else if(strcmp(tmp, "CONNECT") == 0){
            			/*
            			if(login == 0) {
            				fflush(stdout);
            				printf("로그인을 먼저해주세요!\n");
            				break;
            			}
            			*/
            			// printf("connect MODE\n");
            			if(*(tmp + 8) == '['){
            				int index = atoi(tmp + 9);
            				if(*client_list[index].ip == 0){
            					printf("잘못된 index입니다.\n");
            					break;
            				}
            				chat_client_addr.sin_addr.s_addr = inet_addr(client_list[index].ip);
            				printf("%s\n", client_list[index].ip);
            			}
            			else{
            				tmp = strtok(0, " ");
	            			tmp[strlen(tmp)-1] = 0;
	            			chat_client_addr.sin_addr.s_addr = inet_addr(tmp);
            			}
            			chat_client_addr.sin_family = AF_INET;
            			chat_client_addr.sin_port = htons(atoi(argv[2]));
            			// printf("connect to : %s port : %d\n", tmp, atoi(argv[2]));
            			
            			if(connect(tcp_client_socket, (struct sockaddr*) &chat_client_addr, sizeof(chat_client_addr)) == -1){
            				perror("connect error\n");
	            			close(tcp_client_socket);
	            			exit(1);
            			}
            			printf("연결 완료\n");
            			client.chat = '2';
            			is_server = 0;
            			FD_SET(tcp_client_socket, &read_tmp);

            		}
            		else if(strcmp(tmp, "DISCONNECT\n") == 0){
            			printf("활성화 되어있는 연결을 끊습니다.\n");
            			if(is_server == -1){
            				printf("활성화 되어있는 연결이 없습니다.\n");
            				continue;
            			}
            			else if(is_server == 0){
            				close(tcp_client_socket);
            				tcp_server_socket = socket(PF_INET, SOCK_STREAM, 0);
            				FD_CLR(tcp_client_socket, &read_tmp);
            				client.chat = '1';
            				is_server = -1;
            			}
            			else if(is_server == 1){
            				close(tcp_server_socket2);
            				FD_CLR(tcp_server_socket2, &read_tmp);
            				maxfd--;
            				client.chat = '1';
            				is_server = -1;
            			}
            			first_connect = 0;

            		}
            		else if(strcmp(tmp, "CHAT") == 0){
            			tmp = strtok(0, " ");
            			// printf("chat MODE\n");
            			if(is_server == -1){
            				printf("누구와도 연결이 안되었습니다.\n");
            				continue;
            			} 
            			if(is_server == 0) {
            				memset(buf, 0, sizeof(buf));
            				sprintf(buf, "%s : %s", my_nick_name, tmp);
            				write(tcp_client_socket, buf, strlen(buf));

            			}

            			if(is_server == 1) {
            				memset(buf, 0, sizeof(buf));
            				sprintf(buf, "%s : %s", my_nick_name, tmp);
            				write(tcp_server_socket2, buf, strlen(buf));
            			}
            		}
            		else if(strcmp(tmp, "LOGIN") == 0){
            			if(login == 1) break;
            			printf("로그인 시도 중...\n");
            			memset(buf, 0, sizeof(buf));
            			tmp = strtok(0," ");
            			tmp2 = tmp + strlen(tmp) + 1;
            			tmp2[strlen(tmp2) - 1] = 0;
            			sprintf(buf, "%s %s %s ", argv[3],tmp, tmp2);
            			// printf("%s\n", buf);

            			memset(&udp_login_adr, 0, sizeof(udp_login_adr));
        				udp_login_adr.sin_family = AF_INET;
	            		udp_login_adr.sin_port = htons(chat_m_port + 2);
	            		udp_login_adr.sin_addr.s_addr = inet_addr(chat_m_ip);

	            		if(-1 == connect(login_socket, (struct sockaddr*) &udp_login_adr, sizeof(udp_login_adr))){
	            			printf("UDP login server connect error\n");
	            			close(hb_sock);
	            			exit(1);
	        			}

	        			// printf("CONNECT success\n");
	            		str_len = sendto(login_socket, buf, strlen(buf), 0, (struct sockaddr *)&udp_login_adr, udp_info_r_adr_sz);
	            		// printf("SEND LOGIN cmd success\n");
	            		memset(buf, 0, sizeof(buf));
	            		str_len = recvfrom(login_socket,buf, BUF_SIZE -1, 0, (struct sockaddr *)&udp_login_r_adr, &udp_info_r_adr_sz);
	            		// printf("recv success \n");
	            		if(strncmp(buf, "fail", 4) == 0) printf("Login Fail\n");
	            		else {
	            			// printf("%s\n", buf);
	            			printf("%s님 환영합니다. \n", buf + 8);
	            			strcpy(my_nick_name, buf + 8);
	            			login = 1;
	            		}

            		}
            		
            		/**/
            		else if(strcmp(tmp, "DEBUG\n") == 0){
            			printf("is_server : %d \n", is_server);
            			printf("client.chat : %d \n", client.chat);
            			printf("first_connect : %d \n", first_connect);
            			printf("maxfd : %d \n", maxfd);
            			printf("login : %d \n", login);
            		}

            		else{
            			printf("알 수 없는 명령어 입니다. \n");
            			printf("명령어 종류 \n");
            			printf("1. CMD \n");
            			printf("2. CONNECT \n");
            			printf("3. DISCONNECT \n");
            			printf("3. CHAT \n");
            		}
            	}	


		}
		readfds = read_tmp;
	}


}