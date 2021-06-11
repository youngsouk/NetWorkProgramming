#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 55

int main(int argc, char *argv[])
{
	int recv_sock;
	int str_len;
	char buf[8][BUF_SIZE];
	struct sockaddr_in addr;
	struct ip_mreq join_addr;
	char seq[50];
	char tmp[4];
	int r_seq;

	FILE * fp = fopen("1.txt", "wb+");;

	
	if(argc!=3) {
		printf("Usage : %s <GroupIP> <PORT>\n", argv[0]);
		exit(1);
	 }
 
       // Create UDP Socket	
	recv_sock=socket(PF_INET, SOCK_DGRAM, 0);
 	memset(&addr, 0, sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=htonl(INADDR_ANY);	
	addr.sin_port=htons(atoi(argv[2]));

	int on=1;
    setsockopt(recv_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

// Bind socket	
	if(bind(recv_sock, (struct sockaddr*) &addr, sizeof(addr))==-1)
	{
		printf("bind() error");
		close(recv_sock);
		exit(1);	
	}

	// Specify the multicast Group	
	join_addr.imr_multiaddr.s_addr=inet_addr(argv[1]);
	// Accept multicast from any interface
	join_addr.imr_interface.s_addr=htonl(INADDR_ANY);
  	
// Join Multicast Group	
	if ( (setsockopt(recv_sock, IPPROTO_IP, 
		IP_ADD_MEMBERSHIP, (void*)&join_addr, sizeof(join_addr)))< 0 ) 
	{
		printf(" SetsockOpt Join Error \n");
		close(recv_sock);
		exit(1);
	}
  	
	memset(buf, 0, sizeof(buf));
  	for(int i = 0; i < 7; i++){
  		memset(seq, 0, 50);
  		str_len = recvfrom(recv_sock, seq, 50, 0, NULL, 0);
  		memcpy(tmp, seq, 4);
  		r_seq = atoi(tmp);
  		memcpy(buf[r_seq], seq+4, str_len - 4);
		printf("%d %s\n", r_seq, buf[r_seq]);
		
	}
	for(int i = 0; i < 7; i++){
		printf("%s\n", buf[i]);
		fwrite(buf[i], 1, strlen(buf[i]) + 1, fp);
		fwrite("\n", 1, 1, fp);
	}

	close(recv_sock);
	return 0;
}

