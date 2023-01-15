#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#define MAX_SIZE 50

int main(){

    int sockfd, newsockfd;
    int clilen;
    int i;
    struct sockaddr_in servaddr, cliaddr;
    char buf[MAX_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0){
        printf("Server socket creation error\n");
        exit(EXIT_FAILURE);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;    
    servaddr.sin_port = htons(20000);

    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
        printf("Could not bind local address\n");
        exit(EXIT_FAILURE);
    }

    listen(sockfd, 5);

    while (1){

        clilen = sizeof(cliaddr);

        newsockfd = accept(sockfd, (struct sockaddr*)&cliaddr, &clilen);

        if (newsockfd < 0){
            printf("Error while accepting connection request\n");
            exit(EXIT_FAILURE);
        }

        if (fork() == 0){

            close(sockfd);

            for (i=0;i<MAX_SIZE;i++)    buf[i] = '\0';

            strcpy(buf, "LOGIN:");
            send(newsockfd, buf, strlen(buf) + 1, 0);

            for (i=0;i<MAX_SIZE;i++)    buf[i] = '\0';

            recv(newsockfd, buf, MAX_SIZE, 0);
            printf("Username : %s\n", buf);
            
            close(newsockfd);
            exit(0);
        }

        close(newsockfd);
    }
    return 0;
}