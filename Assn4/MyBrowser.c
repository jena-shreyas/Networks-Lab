#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <time.h>
#define CMD_SIZE 10
#define BUF_SIZE 100
#define URL_SIZE 300
#define MAX_SIZE 2048
#define TIMEOUT 3000

typedef struct message_
{
    char cmd[CMD_SIZE];
    char url[URL_SIZE];
    char host[URL_SIZE];
    char ip[URL_SIZE];
    unsigned int port;
    char filename[URL_SIZE];
    char extension[URL_SIZE];
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
    // printf("cmd: %s\n", req.cmd);

    if (!strcmp(req.cmd, "GET"))
    {     
        char *url = strtok(NULL, " ");
        strcpy(req.url, url);
        printf("url: %s\n", req.url);
        char *name_beg = strrchr(url, '/') + sizeof(char);
        char *dot_beg = strrchr(url, '.');
        char *port_beg = strrchr(url, ':');

        if (port_beg != NULL && port_beg > dot_beg)
        {
            strncpy(req.filename, name_beg, port_beg - name_beg);
            req.filename[port_beg - name_beg] = '\0';
            req.port = atoi(port_beg + 1);
        }
        else
        {
            strcpy(req.filename, name_beg);
            req.port = 80;
        }

        char *ext_beg = strrchr(req.filename, '.');
        if (ext_beg != NULL)
            strcpy(req.extension, ext_beg);
        else
            strcpy(req.extension, "");

        char *protocol = strtok(url, ":");
        char *path_port = strtok(NULL, "");

        if (strchr(path_port, ':') != NULL)
            strtok(path_port, ":");

        int idx = 2;
        while (path_port[idx] != '/')
        {
            req.host[idx - 2] = path_port[idx];
            idx++;
        }
        req.host[idx - 2] = '\0';
    }

    else if (!strcmp(req.cmd, "PUT"))
    {
        char *url_file = strtok(NULL, " ");
        char *filename = strtok(NULL, " ");
        strcpy(req.filename, filename);
        // printf("PUT filename: %s\n", req.filename);

        strncpy(req.url, url_file, filename - url_file - 1);
        req.url[filename - url_file - 1] = '\0';
        char *slash_beg = strrchr(req.url, '/');
        char *port_beg = strrchr(req.url, ':');

        if (port_beg != NULL && port_beg > slash_beg)
        {
            req.port = atoi(port_beg + 1);
            req.url[port_beg - req.url] = '\0';
        }
        else
            req.port = 80;

        // printf("PUT url : %s\n", req.url);
        // printf("PUT port : %d\n", req.port);
        char *ext_beg = strrchr(req.filename, '.');
        if (ext_beg != NULL)
            strcpy(req.extension, ext_beg);
        else
            strcpy(req.extension, "");

        char *host_begin = strstr(req.url, "://") + 3 * sizeof(char);
        int idx = 0;

        while (*host_begin != '/' && *host_begin != '\0'){

            req.host[idx++] = *host_begin;
            host_begin += sizeof(char);

        }
        req.host[idx] = '\0';
        // printf("PUT host : %s\n", req.host);
    }

    if (!inet_aton(req.host, NULL))
        strcpy(req.ip, convert_hostname_to_ip(req.host));
    else
        strcpy(req.ip, req.host);

    // printf("PUT IP : %s\n", req.ip);
    return req;
}

int get_status_code(char *response)
{
    if (strstr(response, "200 OK") != NULL)
        return 200;
    else if (strstr(response, "404 Not Found") != NULL)
        return 404;
    else if (strstr(response, "400 Bad Request") != NULL)
        return 400;
    else if (strstr(response, "403 Forbidden") != NULL)
        return 403;
    else
        return 0;
}

void print_status_msgs(int status_code)
{
    if (status_code == 200)
        printf("200 OK\n");
    else if (status_code == 404)
        printf("404 Not Found\n");
    else if (status_code == 400)
        printf("400 Bad Request\n");
    else if (status_code == 403)
        printf("403 Forbidden\n");
    else
        printf("Unknown Error\n");
}

int main()
{
    int sockfd;
    int i;
    struct sockaddr_in servaddr;
    char *buf;
    char input[URL_SIZE];
    char request[MAX_SIZE];
    char *response;
    struct tm *lt;

    const char *prompt = "\nMyBrowser> ";
    buf = (char *)malloc(BUF_SIZE * sizeof(char));

    while (1)
    {

        printf("%s", prompt);
        memset(input, '\0', MAX_SIZE);

        // extract input from user in chunks
        while (1)
        {
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
        char *request = (char *)malloc(MAX_SIZE * sizeof(char));
        if (!strcmp(req.cmd, "GET"))
            sprintf(request, "%s %s HTTP/1.1", req.cmd, req.url);
        else if (!strcmp(req.cmd, "PUT"))
        {
            if (req.url[strlen(req.url) - 1] == '/')
                sprintf(request, "%s %s%s HTTP/1.1", req.cmd, req.url, req.filename);
            else
                sprintf(request, "%s %s/%s HTTP/1.1", req.cmd, req.url, req.filename);
        }
        strcat(request, "\r\nHost: ");
        strcat(request, req.host);
        strcat(request, "\r\nConnection: close");

        time_t t = time(NULL);
        lt = localtime(&t);

        strftime(buf, BUF_SIZE, "%a, %d %b %Y %H:%M:%S %Z", lt);

        strcat(request, "\r\nDate: ");
        strcat(request, buf);

        char accept_type[BUF_SIZE];

        if (req.extension != NULL)
        {
            if (!strcmp(req.extension, ".html"))
                strcpy(accept_type, "text/html");
            else if (!strcmp(req.extension, ".jpg"))
                strcpy(accept_type, "image/jpeg");
            else if (!strcmp(req.extension, ".pdf"))
                strcpy(accept_type, "application/pdf");
            else
                strcpy(accept_type, "text/*");
        }

        if (!strcmp(req.cmd, "GET"))
        {
            strcat(request, "\r\nAccept: ");
            strcat(request, accept_type);
            strcat(request, "\r\nAccept-Language: en-US");

            // lt->tm_mday -= 2;
            // strftime(buf, BUF_SIZE, "%a, %d %b %Y %H:%M:%S %Z", lt);
            // strcat(request, "\r\nIf-Modified-Since: ");
            // strcat(request, buf);

            // add newline at end of request header
            strcat(request, "\r\n\r\n");

            printf("Request : \n\n%s", request);
        }

        else if (!strcmp(req.cmd, "PUT"))
        {
            // add PUT request-specific headers
            strcat(request, "\r\nContent-Language: en-US");
            strcat(request, "\r\nContent-Length: ");
            FILE *fp = fopen(req.filename, "r");
            fseek(fp, 0, SEEK_END);
            int size = ftell(fp);
            fseek(fp, 0, SEEK_SET);

            char size_str[BUF_SIZE];
            sprintf(size_str, "%d", size);
            strcat(request, size_str);
            strcat(request, "\r\nContent-Type: ");
            strcat(request, accept_type);
            strcat(request, "\r\n\r\n");

            // read file content and append to request
            int offset = strlen(request);
            int request_size = MAX_SIZE;
            char *file_content = (char *)malloc(size * sizeof(char));
            fread(file_content, sizeof(char), size, fp);

            for (i = 0; i < size; i++)
            {
                if (offset + i >= request_size)
                {
                    request_size *= 2;
                    request = realloc(request, request_size * sizeof(char));
                }
                request[offset + i] = file_content[i];
            }
            fclose(fp);

            // display request to be sent to server
            printf("Request : \n\n");
            fwrite(request, sizeof(char), offset, stdout);
        }

        // establish connection to server
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("Client socket could not be created!");
            exit(0);
        }

        // connect to server
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(req.port);
        inet_aton(req.ip, &servaddr.sin_addr);

        // servaddr.sin_port = htons(20000);
        // inet_aton("127.0.0.1", &servaddr.sin_addr);

        if ((connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0)
        {
            perror("Client could not connect to server!");
            exit(0);
        }

        char *request_ptr = request;
        //  send input in chunks
        while (1)
        {
            memset(buf, '\0', BUF_SIZE);
            strncpy(buf, request_ptr, BUF_SIZE);
            request_ptr = request_ptr + BUF_SIZE;

            send(sockfd, buf, BUF_SIZE, 0);

            if (strlen(buf) < BUF_SIZE)
                break;
        }

        // receive response from server
        response = (char *)malloc(MAX_SIZE * sizeof(char));
        int response_size = MAX_SIZE;
        int bytes_recv = 0;
        int buf_recv;

        struct pollfd fdset[1];
        fdset[0].fd = sockfd;
        fdset[0].events = POLLIN;
        int ret = poll(fdset, 1, TIMEOUT);

        if (ret > 0)
        {
            while (1)
            {
                memset(buf, '\0', BUF_SIZE);
                if ((buf_recv = recv(sockfd, buf, BUF_SIZE, 0)) == 0)
                    break;
                while (response_size <= bytes_recv + buf_recv)
                {
                    response_size *= 2;
                    response = (char *)realloc(response, response_size * sizeof(char));
                }

                // writing response from buffer
                for (int i = 0; i < buf_recv; i++)
                    response[bytes_recv + i] = buf[i];

                bytes_recv += buf_recv;
            }

            char *body_beg_ptr = strstr(response, "\r\n\r\n");
            size_t offset = (body_beg_ptr - response) / sizeof(char);
            printf("Response : \n\n");
            fwrite(response, sizeof(char), offset, stdout);
            printf("\n\n");

            int status_code = get_status_code(response);

            if (!strcmp(req.cmd, "GET"))
            {
                if (status_code == 200)
                {
                    FILE *fp = fopen(req.filename, "w");
                    fwrite(body_beg_ptr + 4, sizeof(char), bytes_recv - (offset + 4), fp);
                    fclose(fp);

                    if (fork() == 0)
                    {
                        close(sockfd);
                        if (!strcmp(req.extension, ".html"))
                            execlp("/usr/bin/firefox", "firefox", req.filename, NULL);
                        else if (!strcmp(req.extension, ".jpg"))
                            execlp("/usr/bin/shotwell", "shotwell", req.filename, NULL);
                        else if (!strcmp(req.extension, ".pdf"))
                            execlp("/usr/bin/xdg-open", "xdg-open", req.filename, NULL);
                        else
                            execlp("/usr/bin/gedit", "gedit", req.filename, NULL);
                    }
                    wait(NULL); // wait for child process to finish
                }
            }

            // else if (!strcmp(req.cmd, "PUT"))
            // {
            //     // printf("Response : \n\n");
            //     // printf("%s", response);
            //     // fwrite(response, sizeof(char), offset, stdout);
            //     // printf("\n\n");
            //     // char *body_beg_ptr = strstr(response, "\r\n\r\n");
            //     // size_t offset = (body_beg_ptr - response) / sizeof(char);
            // }

            print_status_msgs(status_code);
        }
        else if (ret == 0)
            printf("Timeout exceeded\n");
        else
            printf("Error in poll()\n");

        close(sockfd);
    }
    return 0;
}