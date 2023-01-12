/*
    NAME : SHREYAS JENA
    ROLL : 20CS30049
    ASSIGNMENT 1, Q1 (Client)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(){

    int sockfd;
    int i;
    struct sockaddr_in serv_addr;
    char buf[100];

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Client socket could not be created!");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(20000);
    inet_aton("127.0.0.1", &serv_addr.sin_addr);

    if ((connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0){
        perror("Client could not connect to server!");
        exit(0);
    }

    for (i=0; i < 100; i++) buf[i] = '\0';

	recv(sockfd, buf, 100, 0);
	printf("Local server time : %s", buf);
		
	close(sockfd);
	return 0;
}