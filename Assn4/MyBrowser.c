#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#define CMD_SIZE 10
#define BUF_SIZE 100
#define URL_SIZE 300
#define MAX_SIZE 2048

typedef struct request_
{
    char cmd[CMD_SIZE];
    char url[URL_SIZE];
    char host[URL_SIZE];
    unsigned int port;
} request;

const char *days_of_week[] = {"Sun" ,"Mon" ,"Tue" ,"Wed" ,"Thu" ,"Fri" ,"Sat"};
const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

// parse the input string and return a request struct
request parse_request(char *input)
{
    request req;

    char *token = strtok(input, " ");
    strcpy(req.cmd, token);
    token = strtok(NULL, " ");
    
    char *protocol = strtok(token, ":");
    char *path_port = strtok(NULL, "");
    printf("%s\n", path_port);

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
        req.host[idx - 2] = path_port[idx];
        idx++;
    }

    req.host[idx - 2] = '\0';
    strcpy(req.url, path_port + idx);

    return req;
}

int main(){

    int sockfd;
    int i;
    struct sockaddr_in servaddr;
    char *buf;
    char input[URL_SIZE];
    struct tm* lt;

    const char *prompt = "MyBrowser> ";
    buf = (char *)malloc(BUF_SIZE * sizeof(char));

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
        printf("Host : %s\n", req.host);
        printf("Port : %d\n", req.port);

        // char ip[100];
        // hostname_to_ip(req.ip, ip);

        // servaddr.sin_family = AF_INET;
        // servaddr.sin_port = htons(req.port);
        // inet_aton(req.ip, &servaddr.sin_addr);

        // if ((connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) < 0)
        // {
        //     // printf("%d\n", errno);
        //     perror("Client could not connect to server!");
        //     exit(0);
        // }

        // printf("Connected to server %s : %d\n", req.ip, req.port);

        char request[MAX_SIZE];

        if (!strcmp(req.cmd, "GET"))
        {
            sprintf(request, "GET %s HTTP/1.1", req.url);
            strcat(request, "\nHost: ");
            strcat(request, req.host);
            strcat(request, "\nConnection: close");

            /// ################### CHANGE DATE FORMAT A BIT ###################
            time_t t = time(NULL);
            lt = localtime(&t);

            char hour[3], min[3], sec[3];
            if (lt->tm_hour < 10)
                sprintf(hour, "0%d", lt->tm_hour);
            else
                sprintf(hour, "%d", lt->tm_hour);

            if (lt->tm_min < 10)
                sprintf(min, "0%d", lt->tm_min);
            else
                sprintf(min, "%d", lt->tm_min);

            if (lt->tm_sec < 10)
                sprintf(sec, "0%d", lt->tm_sec);
            else
                sprintf(sec, "%d", lt->tm_sec);

            sprintf(buf, "%s, %d %s %d %s:%s:%s GMT" \
                , days_of_week[lt->tm_wday]
                , lt->tm_mday
                , months[lt->tm_mon]
                , (lt->tm_year + 1900)
                , hour
                , min
                , sec);

            strcat(request, "\nDate: ");
            strcat(request, buf);

            strcat(request, "\nAccept: ");
            char *extension = strrchr(req.url, '.');
            if (extension != NULL)
            {
                if (!strcmp(extension, ".html"))
                    strcat(request, "text/html");
                else if (!strcmp(extension, ".jpg"))
                    strcat(request, "image/jpeg");
                else if (!strcmp(extension, ".pdf"))
                    strcat(request, "application/pdf");
                else
                    strcat(request, "text/*");
            }

            strcat(request, "\nAccept-Language: en-US,en;q=0.5");
            // strcat(request, "\nIf-Modified-Since: ");
            printf("Request : \n\n%s\n", request);
        }

    }
    return 0;
}