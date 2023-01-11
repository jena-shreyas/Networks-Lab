#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define CHUNK_SIZE 5
#define RESULT_SIZE 20

int main(){

    int sockfd;
    struct sockaddr_in serv_addr;
    char* buf;
    long int sent_chunks;
    char result[RESULT_SIZE];

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Client socket could not be created!");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(20000);
    inet_aton("127.0.0.1", &serv_addr.sin_addr);

    // Connect to server
    if ((connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0){
        perror("Client could not connect to server!");
        exit(0);
    }

    while (1){

        buf = (char *)malloc((CHUNK_SIZE + 1) * sizeof(char));
        sent_chunks = 0;
        printf("Enter a valid arithmetic expression : ");

        while(1){
            
            fgets(buf, CHUNK_SIZE + 1, stdin);
            // printf("Received : %s\n", buf);
            char* nline = strchr(buf, '\n');

            if (nline != NULL){

                // printf("End of input detected! \n");
                int idx = (nline - buf) / sizeof(char);
                for (int i = idx; i <= CHUNK_SIZE; i++){
                    buf[i] = '\0';
                }
            }

            send(sockfd, buf, CHUNK_SIZE, 0);     // send chunk to server
            // printf("Sent : %s\n", buf);

            if (sent_chunks == 0 && !strcmp(buf, "-1")){       // if user enters -1, exit

                printf("Process terminated. Closing connection ...");
                close(sockfd);                    // close the connection after user terminates
                exit(0);
            }

            if (nline != NULL)                    // last chunk has been extracted
                break;

            sent_chunks++;

        }

        // receive result of expression from server
        recv(sockfd, result, RESULT_SIZE, 0);
        printf("Result = %s\n", result);

        free(buf);

    }
	return 0;
}