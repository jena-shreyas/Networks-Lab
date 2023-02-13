#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#define CMD_SIZE 10
#define MAX_BUF_SIZE 100
#define URL_SIZE 300
#define MAX_SIZE 2048


typedef struct message_
{
    char cmd[CMD_SIZE];
    char url[URL_SIZE];
    char host[URL_SIZE];
    char ip[URL_SIZE];
    unsigned int port;
} Message;

// convert hostname to IP address
char *convert_hostname_to_ip(char *host)
{
    struct hostent *hent;
    struct in_addr **addr_list;
    int i;

    if ((hent = gethostbyname(host)) == NULL)
    {
        herror("gethostbyname");
        return NULL;
    }

    addr_list = (struct in_addr **)hent->h_addr_list;

    for (i = 0; addr_list[i] != NULL; i++)
        return inet_ntoa(*addr_list[i]);

    return NULL;
}

int recv_request(int newsockfd, char *request){

    // char *request = (char *)malloc(MAX_BUF_SIZE*sizeof(char));
    char buffer[MAX_BUF_SIZE];
    int bytes_recv = 0, total_bytes_recv = 0, i = 0, curr_req_len = MAX_BUF_SIZE;
    memset(buffer, 0, MAX_BUF_SIZE);
    memset(request, 0, MAX_BUF_SIZE);

    // receive request from the client
    while(1){
        bytes_recv = recv(newsockfd, buffer, MAX_BUF_SIZE, 0);

        if(bytes_recv < 0){
            perror("Unable to receive data from the server: recv() call failed !\n");
            return -1;
            // exit(0);
        }

        if (bytes_recv == 0){
            printf("The server S1 has closed the connection.\n");
            close(newsockfd);
            return -1;
            // exit(0);
        }

        total_bytes_recv += bytes_recv;

        for(int j = 0; j < bytes_recv; j++){
            request[i++] = buffer[j];
            if (buffer[j] == '\0')
                break;

            if (i == curr_req_len){
                curr_req_len += MAX_BUF_SIZE;
                request = (char *)realloc(request, curr_req_len*sizeof(char));
            }
        }

        if(request[i-1] == '\0')
            break;
    }

    return 0;
}


int main(){

    int sockfd, newsockfd;
    struct sockaddr_in server_addr, client_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Cannot create the socket: socket() system call failed\n");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Cannot bind the socket: bind() system call failed\n");
        exit(1);
    }

    printf("Server is running on port 8080.\n");

    listen(sockfd, 10);

    int client_addr_len;
    char buffer[MAX_BUF_SIZE];

    while(1){
        client_addr_len = sizeof(client_addr);
        

        if ( (newsockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0)
        {
            perror("Cannot accept the client's connection: accept() system call failed\n");
            exit(1);
        }

        printf("Accepted a connection from a client (%s:%d).\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        if (fork() == 0){

            close(sockfd);

            // receive the request from the client in chunks 
            char *request = (char *)malloc(MAX_BUF_SIZE*sizeof(char));
            int ret = recv_request(newsockfd, request);

            if (ret == -1){
                close(newsockfd);
                exit(0);
            }

            printf("Received request from the client: %s\n", request);

            // parsing the http request to get the command, url, host, port
            

            
        }
    
    }

}