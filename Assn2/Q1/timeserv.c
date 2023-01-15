#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <time.h>
#define MAX_SIZE 100

int main(){

    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    struct tm* local_time;
    char* buf;
    socklen_t len;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0){
        perror("Server socket creation error\n");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8181);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
        perror("Bind error\n");
        exit(EXIT_FAILURE);
    }    

    // initialize a buffer to send and receive data
    // buffer size should be sufficient so as not to drop any packets
    buf = (char *)malloc(MAX_SIZE * sizeof(char));

    while (1){

        // initially accept a message from prospective client to know its address
        strcpy(buf, "Message from server");
        len = sizeof(cliaddr);
        recvfrom(sockfd, buf, MAX_SIZE, 0, (struct sockaddr*)&cliaddr, &len);

        // send server local time to client
        time_t t = time(NULL);
        local_time = localtime(&t);
        buf = asctime(local_time);
        printf("%s\n", buf);

        sendto(sockfd, buf, strlen(buf)+1, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
    }
    
    free(buf);
    close(sockfd);
    return 0;
}