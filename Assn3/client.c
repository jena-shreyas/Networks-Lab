/*
    NAME : SHREYAS JENA
    ROLL : 20CS30049
    ASSIGNMENT 3 (Client)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define BUF_SIZE 50
#define MAX_SIZE 100

int main(int args, char* argv[]){

    int sockfd;
    int i;
    struct sockaddr_in lbaddr;
    char buf[BUF_SIZE];
    char tmp[MAX_SIZE];

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Client socket could not be created!");
        exit(0);
    }

    lbaddr.sin_family = AF_INET;
    lbaddr.sin_port = htons(atoi(argv[1]));
    inet_aton("127.0.0.1", &lbaddr.sin_addr);

    if ((connect(sockfd, (struct sockaddr*)&lbaddr, sizeof(lbaddr))) < 0){
        perror("Client could not connect to load balancer!");
        exit(0);
    }

    memset(buf, '\0', BUF_SIZE);
    memset(tmp, '\0', MAX_SIZE);

    // receive local time from LB in chunks
    while (1){

        memset(buf, '\0', BUF_SIZE);
        recv(sockfd, buf, BUF_SIZE, 0);
        strcat(tmp, buf);

        if (strlen(buf) < (BUF_SIZE - 1))
            break;
    }

	printf("Local server time : %s\n", tmp);
		
	close(sockfd);
	return 0;
}