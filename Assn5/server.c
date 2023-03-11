#include "mysocket.h"

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

    while (1){

        clilen = sizeof(cliaddr);
        if ((newsockfd = my_accept(sockfd, (struct sockaddr*)&cliaddr, &clilen)) < 0) {
            perror("Unable to accept connection!\n");
            exit(EXIT_FAILURE);
        }

        printf("SOCKET WORKING!\n");
    }

    if (my_close(sockfd) < 0) {
            perror("Unable to close socket!\n");
            exit(EXIT_FAILURE);
        }

    return 0;
}