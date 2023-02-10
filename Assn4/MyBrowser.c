#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#define CMD_SIZE 10
#define BUF_SIZE 50
#define MAX_SIZE 1000

typedef struct request_
{
    char cmd[CMD_SIZE];
    char url[MAX_SIZE];
    char ip[MAX_SIZE];
    unsigned int port;
} request;

// parse the input string and return a request struct
request parse_request(char *input)
{
    request req;

    char *token = strtok(input, " ");
    strcpy(req.cmd, token);
    token = strtok(NULL, " ");
    strcpy(req.url, token);
    
    char *protocol = strtok(token, ":");
    char *path_port = strtok(NULL, "");

    if (strchr(path_port, ':') != NULL)
    {
        strtok(path_port, ":");
        req.port = atoi(strtok(NULL, ":"));
    }
    else
        req.port = 80;

    int idx = 2;
    while (path_port[idx] != '/')
    {
        req.ip[idx - 2] = path_port[idx];
        idx++;
    }

    req.ip[idx - 2] = '\0';
    return req;
}

int main(){

    int sockfd;
    int i;
    struct sockaddr_in servaddr;
    char buf[BUF_SIZE];
    char input[MAX_SIZE];

    const char *prompt = "MyBrowser> ";

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Client socket could not be created!");
        exit(0);
    }

    while (1){

        printf("%s", prompt);
        memset(input, '\0', MAX_SIZE);

        // extract input from user in chunks
        while (1){

            memset(buf, '\0', BUF_SIZE);
            fgets(buf, BUF_SIZE, stdin);
            char *npos = strchr(buf, '\n');

            if (npos != NULL)
                *npos = '\0';
               
            strcat(input, buf);
            if (npos != NULL)
                break;
        }

        // if user enters "exit", exit the program
        if (!strcmp(input, "QUIT"))
            break;

        request req = parse_request(input);
        printf("Command : %s\n", req.cmd);
        printf("URL : %s\n", req.url);
        printf("IP : %s\n", req.ip);
        printf("Port : %d\n", req.port);

        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(req.port);
        inet_aton(req.ip, &servaddr.sin_addr);

        if ((connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) < 0)
        {
            // printf("%d\n", errno);
            perror("Client could not connect to server!");
            exit(0);
        }

        printf("Connected to server %s : %d", req.ip, req.port);

    }
    return 0;
}