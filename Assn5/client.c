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
        
    if (my_close(sockfd) < 0) {
        perror("Unable to close socket!\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}