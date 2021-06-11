
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>

#define TTL	2
#define BUF_SIZE 1024


int main(int argc, char *argv[])
{
	int recv_sock;
	int send_sock;
	int time_live = TTL;
	int str_len;
	char buf[BUF_SIZE];
	struct sockaddr_in adr, sadr;
	struct ip_mreq join_adr;
	int on=1;
	
	if(argc!=3) {
		printf("Usage : %s <GroupIP> <PORT>\n", argv[0]);
		exit(1);
	 }
  
// send
	send_sock=socket(PF_INET, SOCK_DGRAM, 0);
 	memset(&adr, 0, sizeof(adr));
	sadr.sin_family=AF_INET;
	sadr.sin_addr.s_addr=inet_addr(argv[1]);
	sadr.sin_port=htons(atoi(argv[2]));

	on=0;
//	setsockopt(send_sock, IPPROTO_IP, IP_MULTICAST_LOOP, &on, sizeof(on));
	setsockopt(send_sock, IPPROTO_IP, 
		IP_MULTICAST_TTL, (void*)&time_live, sizeof(time_live));

	while(1) {
		printf("==> ");
		fgets(buf, BUF_SIZE, stdin);
		sendto(send_sock, buf, BUF_SIZE, 0,(struct sockaddr *)&sadr, sizeof(sadr));
		if (strcmp(buf, "q\n")==0) break;
	}
	printf(" terminate send! \n");
	close(send_sock);
	return 0;
}
