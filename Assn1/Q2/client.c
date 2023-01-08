#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(){

    int sockfd;
    struct sockaddr_in serv_addr;
    char ch;
    size_t len, size;
    char* buf;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Client socket could not be created!");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(20000);
    inet_aton("127.0.0.1", &serv_addr.sin_addr);

    while (1){

        /* 
            For each iteration of client, a new connection is established with the server
            This is done bcoz neither server (nor client) know how many iterations user will access the server
            Granting exclusivity to a particular client in terms of connection time is both unreasonable and harder to implement
        */

        // Connect to server
        if ((connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0){
            perror("Client could not connect to server!");
            exit(0);
        }

        buf = NULL;
        size = 0, len = 0;

        printf("Enter a valid arithmetic expression : ");
        while ((ch = getchar()) != '\n'){

            if (len + 1 >= size){

                size = 2 * size + 1;
                buf = realloc(buf, size);
            }

            buf[len++] = ch;
        }
	
        buf[len] = '\0';

        // if user enters -1, process is terminated
        if (!strcmp(buf, "-1")){

            printf("Process terminated.");
            exit(0);
        }

        send(sockfd, buf, len + 1, 0);
        close(sockfd);  // close the connection after user terminates

    }
	return 0;
}