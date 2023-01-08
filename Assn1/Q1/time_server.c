#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

int main(){

    int sockfd, newsockfd;
    int clilen;
    struct sockaddr_in serv_addr, cli_addr;
    struct tm* local_time;
    char* buf;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Server socket could not be created!");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(20000);

    if ((bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0){

        perror("Could not bind local address to server socket!");
        exit(0);
    }

    listen(sockfd, 5);

    while (1) {

        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);

        if (newsockfd < 0){

            perror("Error while connecting to server!");
            exit(0);
        }

        strcpy(buf, "Message from server");

        time_t t = time(NULL);
        local_time = localtime(&t);
        buf = asctime(local_time);
        printf("%s", buf);

        send(newsockfd, buf, strlen(buf)+1, 0);

	    // recv(newsockfd, buf, 100, 0);
        // printf("%s", buf);

        close(newsockfd);
    }
    return 0;
}