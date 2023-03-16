#include "mysocket.h"
#include<time.h>

int main(){

    int sockfd, newsockfd;
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;
    socklen_t clilen;

    if ((sockfd = my_socket(AF_INET, SOCK_MyTCP, 0)) < 0) {
        perror("Cannot create the socket!\n");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(20000);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if (my_bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Unable to bind local address!\n");
        exit(EXIT_FAILURE);
    }

    if (my_listen(sockfd, 5) < 0) {
        perror("Unable to listen on socket!\n");
        exit(EXIT_FAILURE);
    }

    printf("SOCKET LISTENING!\n");

    clilen = sizeof(cliaddr);
    if ((newsockfd = my_accept(sockfd, (struct sockaddr*)&cliaddr, &clilen)) < 0) {
        perror("Unable to accept connection!\n");
        exit(EXIT_FAILURE);
    }

    printf("Connected to a client!\n");

    char buffer[MAX_MESSAGE_SIZE];
    memset(buffer, 0, MAX_MESSAGE_SIZE);

    size_t len = 0;

    // SENDING MULTIPLE MESSAGES (TESTING)
    
    // for (int i=0;i<15;i++){

    //     if ((len = my_recv(newsockfd, buffer, 5000, 0)) < 0) {
    //         perror("Unable to receive message!\n");
    //         exit(EXIT_FAILURE);
    //     }

    //     printf("Message received from client!\n");
    //     printf("Message length: %ld\n", len);
    //     printf("Message:\n");
    //     for (int i = 0; i < len; i++) {
    //         printf("%c", buffer[i]);
    //     }
    //     printf("\n");
    //     memset(buffer, 0, MAX_MESSAGE_SIZE);
    // }

    if ((len = my_recv(newsockfd, buffer, 4105, 0)) < 0) {
            perror("Unable to receive message!\n");
            exit(EXIT_FAILURE);
        }

        printf("Message received from client!\n");
        printf("Message length: %ld\n", len);
        printf("Message:\n");
        for (int i = 0; i < len; i++) {
            printf("%c", buffer[i]);
        }
        printf("\n");
        memset(buffer, 0, MAX_MESSAGE_SIZE);

    time_t t = time(NULL);
    struct tm* local_time = localtime(&t);
    char *buf;
    buf = asctime(local_time);
    printf("%s", buf);

    printf("Sending message...\n");
    len = my_send(newsockfd, buf, strlen(buf), 0);


    // while (1){

    //     clilen = sizeof(cliaddr);
    //     if ((newsockfd = my_accept(sockfd, (struct sockaddr*)&cliaddr, &clilen)) < 0) {
    //         perror("Unable to accept connection!\n");
    //         exit(EXIT_FAILURE);
    //     }


    //     printf("SOCKET WORKING!\n");

    //     char buffer[MAX_MESSAGE_SIZE];
    //     memset(buffer, 0, MAX_MESSAGE_SIZE);

    //     size_t len = 0;

    //     if ( (len = my_recv(newsockfd, buffer, MAX_MESSAGE_SIZE, 0)) < 0) {
    //         perror("Unable to receive message! hi\n");
    //         exit(EXIT_FAILURE);
    //     }

    //     printf("Message received from client: %s\n", buffer);
    //     printf("Message length: %ld\n", len);
    //     printf("Message:\n");
    //     for (int i = 0; i < len; i++) {
    //         printf("%c", buffer[i]);
    //     }
    //     printf("\n");

    //     if (my_close(newsockfd) < 0) {
    //         perror("Unable to close socket!\n");
    //         exit(EXIT_FAILURE);
    //     }

    // }

    if (my_close(sockfd) < 0) {
            perror("Unable to close socket!\n");
            exit(EXIT_FAILURE);
    }

    return 0;
}