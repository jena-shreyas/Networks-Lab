#include "mysocket.h"

int main(){

    int sockfd;
    struct sockaddr_in servaddr;

    if ((sockfd = my_socket(AF_INET, SOCK_MyTCP, 0)) < 0) {
        perror("Cannot create the socket!\n");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(20000);
    inet_aton("127.0.0.1", &servaddr.sin_addr);

    if (my_connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("Unable to connect!\n");
        exit(EXIT_FAILURE);
    }

    printf("CONNECTED!\n");

    char buffer[MAX_MESSAGE_SIZE];
    memset(buffer, 0, MAX_MESSAGE_SIZE);
    size_t len = 0;

    // copy a random string of size 4000 into buffer
    for (int i = 0; i < 4000; i++) {
        buffer[i] = 'a';
    }

    printf("Sending message...\n");
    len = my_send(sockfd, buffer, 4000, 0);
    
    memset(buffer, 0, MAX_MESSAGE_SIZE);
    
    // size_t len = 0;

    if ( (len = my_recv(sockfd, buffer, MAX_MESSAGE_SIZE, 0)) < 0) {
        perror("Unable to receive message!!\n");
        if (my_close(sockfd) < 0) {
            perror("Unable to close socket!\n");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_FAILURE);
    }
    
    printf("Message received from the server!\n");
    printf("Message length: %ld\n", len);
    printf("Message:\n");
    for (int i = 0; i < len; i++) {
        printf("%c", buffer[i]);
    }
    printf("\n");



    if (my_close(sockfd) < 0) {
        perror("Unable to close socket!\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}