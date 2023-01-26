/*
    NAME : SHREYAS JENA
    ROLL : 20CS30049
    ASSIGNMENT 3 (Server)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#define BUF_SIZE 50
#define MAX_SIZE 100

int main(int args, char* argv[]){

    int sockfd, newsockfd;
    socklen_t lblen;
    struct sockaddr_in servaddr, lbaddr;
    struct tm* local_time;
    char *buf;
    char tmp[MAX_SIZE];

    time_t t_rand;
    srand((unsigned)time(&t_rand));

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Server socket could not be created!");
        exit(0);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(atoi(argv[1]));

    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) < 0){

        perror("Could not bind local address to server socket!");
        exit(0);
    }

    listen(sockfd, 5);
    buf = (char *)malloc(BUF_SIZE * sizeof(char));

    while (1) {

        lblen = sizeof(lbaddr);
        newsockfd = accept(sockfd, (struct sockaddr*)&lbaddr, &lblen);

        if (newsockfd < 0){

            perror("Error while connecting to server!");
            exit(0);
        }

        memset(buf, '\0', BUF_SIZE);
        memset(tmp, '\0', MAX_SIZE);

        // receive load/time request from LB in chunks
        while (1){

            memset(buf, '\0', BUF_SIZE);
            recv(newsockfd, buf, BUF_SIZE, 0);
            strcat(tmp, buf);

            if (strlen(buf) < (BUF_SIZE - 1))
                break;
        }

        printf("Request received: %s\n", tmp);

        // send load to LB
        if (!strcmp(tmp, "Send Load")){

            memset(buf, '\0', BUF_SIZE);
            sprintf(buf, "%d", (rand() % 100 + 1));
            send(newsockfd, buf, strlen(buf)+1, 0);
            printf("Load sent: %s\n", buf);
        }

        // send time to LB
        else if (!strcmp(tmp, "Send Time")){

            time_t t = time(NULL);
            local_time = localtime(&t);
            buf = asctime(local_time);
            printf("Local time: %s\n", buf);
            send(newsockfd, buf, strlen(buf)+1, 0);
        }

        close(newsockfd);
    }

    free(buf);
    return 0;
}