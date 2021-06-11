#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

#define MAX_BUF	255
#define MAX_SOCK 10

int main(int argc, char **argv)
{
    int server_sockfd, client_sockfd, sockfd;
    int state, client_len;
    int i, max_client, maxfd;
    int client[MAX_SOCK];

    struct sockaddr_in clientaddr, serveraddr;
    struct timeval tv;
    fd_set readfds, otherfds, allfds;

    char buf[MAX_BUF];
    char msg[MAX_BUF];

    state = 0;


    if(argc!=2) {
	    printf("Usage : %s <port>\n", argv[0]);
	    exit(1);
    }

    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error : ");
        exit(0);
    }

    memset(&serveraddr, 0x00, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(atoi(argv[1]));

    state = bind(server_sockfd, (struct sockaddr *)&serveraddr,
            sizeof(serveraddr));
    if (state == -1) {
        perror("bind error : ");
        exit(0);
    }

    state = listen(server_sockfd, 5);
    if (state == -1) {
        perror("listen error : ");
        exit(0);
    }

    client_sockfd = server_sockfd;

    // init
    max_client = -1;
    maxfd = server_sockfd;

   for (i = 0; i < MAX_SOCK; i++) {
        client[i] = -1;
    }

    FD_ZERO(&readfds);
    FD_SET(server_sockfd, &readfds);
    //

    printf("\nTCP Server Starting ... \n\n\n");
    fflush(stdout);

    while(1)
    {
        allfds = readfds;
        state = select(maxfd + 1 , &allfds, NULL, NULL, NULL);

	// Server Socket - accept from client
        if (FD_ISSET(server_sockfd, &allfds)) {
            client_len = sizeof(clientaddr);
            client_sockfd = accept(server_sockfd,
                    (struct sockaddr *)&clientaddr, &client_len);
            printf("\nconnection from (%s , %d)",
                    inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port));

            for (i = 0; i < MAX_SOCK; i++)
            {
                if (client[i] < 0) {
                    client[i] = client_sockfd;
                    printf("\nclientNUM=%d\nclientFD=%d\n", i+1, client_sockfd);
                    break;
                }
            }

            printf("accept [%d]\n", client_sockfd);
            printf("===================================\n");
            if (i == MAX_SOCK) {
                perror("too many clients\n");
            }
            FD_SET(client_sockfd,&readfds);

            if (client_sockfd > maxfd)
                maxfd = client_sockfd;

            if (i > max_client)
                max_client = i;

            if (--state <= 0)
                continue;

        }

	// client socket check
        for (i = 0; i <= max_client; i++)
        {
            if ((sockfd = client[i]) < 0) {
                continue;
            }

            if (FD_ISSET(sockfd, &allfds))
            {
                memset(buf, 0, MAX_BUF);

		// Disconnet from Client 
                if (read(sockfd, buf, MAX_BUF) <= 0) {
                    printf("Close sockfd : %d\n",sockfd);
            	    printf("===================================\n");
                    close(sockfd);
                    FD_CLR(sockfd, &readfds);
                    client[i] = -1;
                }   
		else {        
                    memset(msg, 0, MAX_BUF);
               		printf("[%d]RECV: %s\n", sockfd, buf);
                    printf("SEND MSG : "); 
                    scanf("%s", msg);
                	write(sockfd, msg, MAX_BUF);

			}

                if (--state <= 0)
                    break;
            }
        } // for
    } // while
}
