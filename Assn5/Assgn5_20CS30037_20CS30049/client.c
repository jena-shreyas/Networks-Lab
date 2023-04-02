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

    // ACTUAL
    // copy a random string of size 4000 into buffer

    // sending 15 messages for testing
    printf("Sending 15 messages...\n");
    for (int i = 0; i < 15; i++) {
        
        char ch = 'a' + i;
        for (int j = 0; j < 500; j++) {
            buffer[j] = ch;
        }
        len = my_send(sockfd, buffer, 500, 0);
        printf("Message of length %ld sent!\n", len);
        memset(buffer, 0, MAX_MESSAGE_SIZE);
    }

    for (int i = 0; i < 5000; i++) {
        if (i % 10 == 0) 
            buffer[i] = 'a';
        else if (i %10 == 1)
            buffer[i] = 'b';
        else if (i %10 == 2)
            buffer[i] = 'c';
        else if (i %10 == 3)
            buffer[i] = 'd';
        else if (i %10 == 4)
            buffer[i] = 'e';
        else if (i %10 == 5)
            buffer[i] = 'f';
        else if (i %10 == 6)
            buffer[i] = 'g';
        else if (i %10 == 7)
            buffer[i] = 'h';
        else if (i %10 == 8)
            buffer[i] = 'i';
        else if (i %10 == 9)
            buffer[i] = 'j';
    }


    // ACTUAL
    printf("Sending message...\n");
    len = my_send(sockfd, buffer, 5000, 0);
    printf("Message of length %ld sent!\n", len);
    
    memset(buffer, 0, MAX_MESSAGE_SIZE);
    

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


    printf("Closing the socket!\n");
    if (my_close(sockfd) < 0) {
        perror("Unable to close socket!\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}