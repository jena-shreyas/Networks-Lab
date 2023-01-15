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

    int sockfd;
    struct sockaddr_in servaddr;
    int i;
    char buf[MAX_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0){
        printf("Client socket creation error\n");
        exit(EXIT_FAILURE);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(20000);
    inet_aton("127.0.0.1", &servaddr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
        printf("Could not connect to server\n");
        exit(EXIT_FAILURE);
    }

    for(i=0;i<MAX_SIZE;i++) buf[i] = '\0';

    recv(sockfd, buf, MAX_SIZE, 0);
    printf("%s", buf);

    for(i=0;i<MAX_SIZE;i++) buf[i] = '\0';
    scanf("%25s", buf);         

    send(sockfd, buf, 26, 0);                   // send username

    for(i=0;i<MAX_SIZE;i++) buf[i] = '\0';
    recv(sockfd, buf, MAX_SIZE, 0);             // receive username status

    if (!strcmp(buf, "NOT-FOUND")){

        printf("Invalid username\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    else{

        while(1){

            printf("Enter a shell command : ");
            scanf("%s", buf);

            printf("Shell command : %s\n", buf);
            size_t len = strlen(buf);
            buf[len] = '\0';

            send(sockfd, buf, strlen(buf) + 1, 0);    // send shell command to server

            if (!strcmp(buf, "exit")){

                close(sockfd);
                exit(0);
            }

            recv(sockfd, buf, MAX_SIZE, 0);           // receive server response

            if (!strcmp(buf, "$$$$"))
                printf("Invalid command\n");

            else if (!strcmp(buf, "####"))
                printf("Error in running command\n");

            else {
                printf("Command correct, enter another one\n");
                // HANDLE RESULTS RETURNED BY SERVER HERE
            }   
        }
    }
    close(sockfd);
    return 0;

}