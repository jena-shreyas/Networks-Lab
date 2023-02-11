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
#define BUF_SIZE 100
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

// parse the input string and return a request struct
Message parse_request(char *input)
{
    Message req;

    char *token = strtok(input, " ");
    strcpy(req.cmd, token);
    token = strtok(NULL, " ");
    
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
        req.host[idx - 2] = path_port[idx];
        idx++;
    }

    req.host[idx - 2] = '\0';

    if (!inet_aton(req.host, NULL))
        strcpy(req.ip, convert_hostname_to_ip(req.host));
    else
        strcpy(req.ip, req.host);

    strcpy(req.url, path_port + idx);

    return req;
}

// Function to parse the HTTP response (FIX THIS!!!!)
void parse_http_response(char *response) 
{
  char *token;
  char *status_line;
  char *header_line;

  // Get the first line (status line)
  status_line = strtok(response, "\n");

  // Get the HTTP status code
  // ************* FIX THIS *************
  token = strtok(status_line, " ");
  printf("%s\n", token);
  int status_code = atoi(token + strlen("HTTP/1.1"));
  printf("HTTP status code: %d\n", status_code);


  // Get the rest of the headers
  while ((header_line = strtok(NULL, "\n")) != NULL) 
  {
    // Extract the header name and value
    token = strtok(header_line, ": ");
    char *header_name = token;
    char *header_value = strtok(NULL, "\0");
    printf("%s: %s\n", header_name, header_value);
  }

  // ************* FIX THIS *************
}

int main(){

    int sockfd;
    int i;
    struct sockaddr_in servaddr;
    char *buf;
    char input[URL_SIZE];
    char request[MAX_SIZE];
    char response[MAX_SIZE];
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

        Message req = parse_request(input);
        printf("Command : %s\n", req.cmd);
        printf("URL : %s\n", req.url);
        printf("Host : %s\n", req.host);
        printf("Port : %d\n", req.port);
        printf("IP : %s\n", req.ip);

        sprintf(request, "%s %s HTTP/1.1", req.cmd, req.url);
        strcat(request, "\nHost: ");
        strcat(request, req.host);
        strcat(request, "\nConnection: close");

        time_t t = time(NULL);
        lt = localtime(&t);

        strftime(buf, BUF_SIZE, "%a, %d %b %Y %H:%M:%S %Z", lt);

        strcat(request, "\nDate: ");
        strcat(request, buf);

        char *extension = strrchr(req.url, '.');
        char accept_type[BUF_SIZE];

        if (extension != NULL)
        {
            if (!strcmp(extension, ".html"))
                strcpy(accept_type, "text/html");
            else if (!strcmp(extension, ".jpg"))
                strcpy(accept_type, "image/jpeg");
            else if (!strcmp(extension, ".pdf"))
                strcpy(accept_type, "application/pdf");
            else
                strcpy(accept_type, "text/*");
        }

        if (!strcmp(req.cmd, "GET"))
        {
            strcat(request, "\nAccept: ");
            strcat(request, accept_type);
            strcat(request, "\nAccept-Language: en-US");

            lt->tm_mday -= 2;
            strftime(buf, BUF_SIZE, "%a, %d %b %Y %H:%M:%S %Z", lt);
            strcat(request, "\nIf-Modified-Since: ");
            strcat(request, buf);

            printf("Request : \n\n%s\n", request);
        }

        else if (!strcmp(req.cmd, "PUT"))
        {
            strcat(request, "\nContent-Language: en-US");
            strcat(request, "\nContent-Length: ");
            FILE *fp = fopen((req.url + 1), "r");
            fseek(fp, 0, SEEK_END);
            int size = ftell(fp);
            fseek(fp, 0, SEEK_SET);

            char size_str[BUF_SIZE];
            sprintf(size_str, "%d", size);
            strcat(request, size_str);

            strcat(request, "\nContent-Type: ");
            strcat(request, accept_type);

            strcat(request, "\n\n");

            char *file_content = (char *)malloc(size * sizeof(char));
            fread(file_content, sizeof(char), size, fp);
            strcat(request, file_content);
            fclose(fp);

            printf("Request : \n\n%s\n", request);
        }

        // // connect to server
        // servaddr.sin_family = AF_INET;
        // servaddr.sin_port = htons(req.port);
        // inet_aton(req.ip, &servaddr.sin_addr);

        // if ((connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) < 0)
        // {
        //     // printf("%d\n", errno);
        //     perror("Client could not connect to server!");
        //     exit(0);
        // }

        // // printf("Connected to server %s : %d\n", req.ip, req.port);

        // char *request_ptr = request;
        // //  send input in chunks
        // while (1){

        //     memset(buf, '\0', BUF_SIZE);
        //     strncpy(buf, request_ptr, BUF_SIZE);
        //     request_ptr = request_ptr + BUF_SIZE;

        //     send(sockfd, buf, BUF_SIZE, 0);

        //     if (strlen(buf) < BUF_SIZE)
        //         break;
        // }

        memset(response, '\0', MAX_SIZE);

        // ***********SAMPLE TESTING OF PARSE_RESPONSE***********
        strcpy(response, "HTTP/1.1 200 OK\n");
        strcat(response, "Content-Type: text/html\n");
        strcat(response, "Content-Length: 42\n");
        strcat(response, "Connection: close\n");
        strcat(response, "\n");
        // receive response from server

        // while (1){

        //     memset(buf, '\0', BUF_SIZE);
        //     recv(sockfd, buf, BUF_SIZE, 0);
        //     strcat(response, buf);

        //     if (strlen(buf) < BUF_SIZE)
        //         break;
        // }

        // printf("Response : \n\n%s\n\n", response);

        // parse response
        char *response_ptr = response;
        memset(buf, '\0', BUF_SIZE);

        parse_http_response(response_ptr);
    }
    return 0;
}