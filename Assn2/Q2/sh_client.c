#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#define BUF_SIZE 50
#define MAX_SIZE 200

int main(){

    int sockfd;
    struct sockaddr_in servaddr;
    int i;
    char buf[BUF_SIZE];
    char input[MAX_SIZE];

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

    for(i=0;i<BUF_SIZE;i++) buf[i] = '\0';

    // receive login prompt from server
    recv(sockfd, buf, BUF_SIZE, 0);
    printf("%s", buf);

    // scan username from user
    for(i=0;i<BUF_SIZE;i++) buf[i] = '\0';
    scanf("%25s", buf);         
    getchar();                                  // To consume \n at end

    // send username
    send(sockfd, buf, 26, 0);                   

    // receive username status
    for(i=0;i<BUF_SIZE;i++) buf[i] = '\0';
    recv(sockfd, buf, BUF_SIZE, 0);             

    // invalid username
    if (!strcmp(buf, "NOT-FOUND")){

        printf("Invalid username\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    else {

        while(1){

            printf("Enter a shell command : ");
            fgets(input, MAX_SIZE, stdin);
            input[strlen(input) - 1] = '\0';        // null-terminating the input

            int i = 0, num_chars = 0;

            // clearing buffer
            for (int j = 0; j<BUF_SIZE; j++)    buf[j] = '\0';

            // sending input in buffer-sized chunks
            while (input[i] != '\0'){

                buf[num_chars++] = input[i++];

                if (i % (BUF_SIZE - 1) == 0){

                    send(sockfd, buf, strlen(buf) + 1, 0);

                    for (int j = 0; j < BUF_SIZE; j++)    buf[j] = '\0';      // clear buffer
                    num_chars = 0;

                }
            }

            // send the last packet
            send(sockfd, buf, strlen(buf) + 1, 0);      

            // if cmd = exit, close socket and exit
            if (!strcmp(input, "exit")){

                close(sockfd);
                exit(0);
            }

            // clear the input for storing result
            for (int j = 0; j < MAX_SIZE; j++)   input[j] = '\0';

            // Input chunk receiving and assembling
            while (1){

                recv(sockfd, buf, BUF_SIZE, 0);
                strcat(input, buf);

                if (strlen(buf) < (BUF_SIZE - 1))
                    break;
            }

            if (!strcmp(input, "$$$$"))
                printf("\nInvalid command\n\n");

            else if (!strcmp(input, "####"))
                printf("\nError in running command\n\n");

            else 
                printf("\nResult : \n%s\n\n", input);
        }
    }

    close(sockfd);
    return 0;

}