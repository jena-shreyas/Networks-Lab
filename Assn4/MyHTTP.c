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
#include <errno.h>
#include <sys/stat.h>

#define CMD_SIZE 10
#define MAX_BUF_SIZE 2048
#define URL_SIZE 600
#define MAX_SIZE 2048


typedef struct message_
{
    char cmd[CMD_SIZE];
    char file_path[URL_SIZE];
    char host[URL_SIZE];
    char ip[URL_SIZE];
    char conn_type[MAX_BUF_SIZE];
    char accept_type[MAX_BUF_SIZE];
    char accept_lang[MAX_BUF_SIZE];
    unsigned int port;
    char if_mod_since[URL_SIZE];
    char content_lang[MAX_BUF_SIZE];
    char content_type[MAX_BUF_SIZE];
    long long int content_len;
} Message;


Message *init_message(){
    Message *msg = (Message *)malloc(sizeof(Message));
    memset(msg->cmd, 0, CMD_SIZE);
    memset(msg->file_path, 0, URL_SIZE);
    memset(msg->host, 0, URL_SIZE);
    memset(msg->ip, 0, URL_SIZE);
    memset(msg->conn_type, 0, MAX_BUF_SIZE);
    memset(msg->accept_type, 0, URL_SIZE);
    memset(msg->accept_lang, 0, MAX_BUF_SIZE);
    memset(msg->if_mod_since, 0, URL_SIZE);
    memset(msg->content_lang, 0, MAX_BUF_SIZE);
    memset(msg->content_type, 0, MAX_BUF_SIZE);
    msg->content_len = 0;
    msg->port = 0;
    return msg;
}

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

// int recv_request(int newsockfd, char *request){

//     // char *request = (char *)malloc(MAX_BUF_SIZE*sizeof(char));
//     char buffer[MAX_BUF_SIZE];
//     int bytes_recv = 0, total_bytes_recv = 0, curr_req_len = MAX_BUF_SIZE;
//     long long int i = 0;
//     memset(request, 0, MAX_BUF_SIZE);

//     // receive request from the client
//     while(1){

//         // memset(buffer, 0, MAX_BUF_SIZE);
//         bytes_recv = recv(newsockfd, buffer, MAX_BUF_SIZE, 0);

//         if(bytes_recv < 0){
//             perror("Unable to receive data from the server: recv() call failed !\n");
//             return -1;
//             // exit(0);
//         }

//         if (bytes_recv == 0){
//             // printf("The server S1 has closed the connection.\n");
//             // close(newsockfd);
//             // return -1;
//             break;
//             // exit(0)
//         }

//         total_bytes_recv += bytes_recv;

//         for(int j = 0; j < bytes_recv; j++){
//             request[i++] = buffer[j];
//             // if (buffer[j] == '\0')
//             //     break;

//             if (i == curr_req_len){
//                 curr_req_len *= 2;
//                 request = (char *)realloc(request, curr_req_len*sizeof(char));
//             }
//         }

//         // while (curr_req_len <= total_bytes_recv + bytes_recv){
//         //     curr_req_len *= 2;
//         //     request = (char *)realloc(request, curr_req_len*sizeof(char));
//         // }


//         // for(int j = 0; j < bytes_recv; j++){
//         //     request[total_bytes_recv + j] = buffer[j];


//         //     // if (i == curr_req_len){
//         //     //     curr_req_len *= 2;
//         //     //     request = (char *)realloc(request, curr_req_len*sizeof(char));
//         //     // }
//         // }
//         // total_bytes_recv += bytes_recv;

//     }
//     return 0;
// }

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
            // printf("The server S1 has closed the connection.\n");
            // close(newsockfd);
            // return -1;
            // exit(0)
            break;
        }

        total_bytes_recv += bytes_recv;

        for(int j = 0; j < bytes_recv; j++){
            request[i++] = buffer[j];
            // if (buffer[j] == '\0')
            //     break;

            if (i == curr_req_len){
                curr_req_len += MAX_BUF_SIZE;
                request = (char *)realloc(request, curr_req_len*sizeof(char));
            }
        }

        if(request[i-1] == '\0')
            // continue;
            break;
    }
    return 0;
}

int parse_request(char *request, Message *msg){

    int cmd_flag = 0, accept_type_flag = 0, conn_type_flag = 0, host_flag = 0;
 
    char **tokens = (char **)malloc(10*sizeof(char *));

    // first tokenise the indivdual headers
    char *temp = strtok(request, "\r\n");
    int i = 0;

    if (temp == NULL){ // headers are not properly seperated by \r\n since not a single token is found after tokenising by \r\n
        printf("The headers are not properly formatted.\n");
        return -1;
    }

    while(temp != NULL){
        tokens[i++] = temp;
        temp = strtok(NULL, "\r\n");
    }

    for (int j = 0; j < i; j++){
        printf("i: %d, %s\n", j, tokens[j]);
    }


    for (int j = 0; j < i; j++){

        // identify the header with the command (GET or PUT)

        if ( strstr(tokens[j], "GET") != NULL || strstr(tokens[j], "PUT") != NULL){
            char *token = strtok(tokens[j], " ");

            strcpy(msg->cmd, token);
            printf("Command: %s\n", msg->cmd);
            cmd_flag = 1;

            // now get the url
            token = strtok(NULL, " ");

            // to get file name, get the position of first '/'
            char *p = strstr(token, "//") + 2;
            token = strtok(p, "/");
            strcpy(msg->ip, convert_hostname_to_ip(token));
            token = strtok(NULL, ":");
            strcpy(msg->file_path, token);
            printf("File path: %s\n", msg->file_path);

            // now get the port number
            if ( (token = strtok(NULL, ":")) == NULL)
                msg->port = 80;
            else
                msg->port = atoi(token);
            printf("Port: %d\n", msg->port);
        }

        // identify the header with the host
        else if ( strstr(tokens[j], "Host") != NULL){
            char *token = strtok(tokens[j], " ");
            token = strtok(NULL, " ");
            strcpy(msg->host, token);
            printf("Host: %s\n", msg->host);
            host_flag = 1;
        }

        // identify the header with the connection type (keep-alive or close)
        else if ( strstr(tokens[j], "Connection") != NULL){
            char *token = strtok(tokens[j], " ");
            token = strtok(NULL, " ");
            strcpy(msg->conn_type, token);
            printf("Connection type: %s\n", msg->conn_type);

            if (strcmp(msg->conn_type, "keep-alive") != 0 || strcmp(msg->conn_type, "close") != 0){
                printf("The connection type is supported by the server.\n");
                conn_type_flag = 1;
            }

        }

        // get the accept type
        else if ( strstr(tokens[j], "Accept:") != NULL){
            char *token = strtok(tokens[j], " ");
            token = strtok(NULL, " ");
            strcpy(msg->accept_type, token);
            // printf("Accept type: %s\n", msg->accept_type);

            if (strcmp(msg->accept_type, "text/html") != 0 || strcmp(msg->accept_type, "application/pdf") != 0 || strcmp(msg->accept_type, "image/jpeg") != 0 || strcmp(msg->accept_type, "text/*") != 0){
                printf("The requested file type is supported by the server.\n");
                accept_type_flag = 1;
            }
        }

        // get the accept language
        else if ( strstr(tokens[j], "Accept-Language") != NULL){
            char *token = strtok(tokens[j], " ");
            token = strtok(NULL, " ");
            strcpy(msg->accept_lang, token);
            // printf("Accept language: %s\n", msg->accept_lang);
        }

        // get the if-modified-since header
        else if ( strstr(tokens[j], "If-Modified-Since") != NULL){
            char *token = strtok(tokens[j], " ");
            token = strtok(NULL, "\r");
            strcpy(msg->if_mod_since, token);
            // printf("If modified since: %s\n", msg->if_mod_since);
        }

        else if ( strstr(tokens[j], "Content-Language") != NULL){
            char *token = strtok(tokens[j], " ");
            token = strtok(NULL, " ");
            strcpy(msg->content_lang, token);
            printf("Content language: %s\n", msg->content_lang);
        }

        else if ( strstr(tokens[j], "Content-Length") != NULL){
            char *token = strtok(tokens[j], " ");
            token = strtok(NULL, " ");
            msg->content_len = atoi(token);
            printf("Content length: %ld\n", msg->content_len);
        }

        else if ( strstr(tokens[j], "Content-Type") != NULL){
            char *token = strtok(tokens[j], " ");
            token = strtok(NULL, " ");
            strcpy(msg->content_type, token);
            printf("Content type: %s\n", msg->content_type);
        }
    }

    if (cmd_flag  == 1 && host_flag == 1 && conn_type_flag == 1 && accept_type_flag == 1){
        printf("The request is valid.\n");
        return 0;
    }
    else{
        printf("The request is invalid.\n");
        return -1;
    }

}

void add_standard_headers(char *response){

    // send the connection type to the client
    strcat(response, "\r\nConnection: ");
    strcat(response, "close");

    // send the cache control to the client
    strcat(response, "\r\nCache-Control: ");
    strcat(response, "no-cache");

    // send the expires time to the client
    strcat(response, "\r\nExpires: ");

    // set it to the current time + 3 days
    time_t curr_time = time(NULL);
    struct tm* curr_local_time_struct = localtime(&curr_time);
    curr_local_time_struct->tm_mday += 3;

    char * time_str = (char *)malloc(100*sizeof(char));
    strftime(time_str, 100, "%a, %d %b %Y %H:%M:%S %Z", curr_local_time_struct);
    strcat(response, time_str);

    return;

}

void add_date_server_name_headers(char *response){

    // send the current date and time (local time) to the client
    time_t curr_time = time(NULL);
    struct tm *curr_local_time_struct = localtime(&curr_time);

    char *time_str = (char *)malloc(100*sizeof(char));
    strftime(time_str, 100, "%a, %d %b %Y %H:%M:%S %Z", curr_local_time_struct);
    strcat(response, "\r\nDate: ");
    strcat(response, time_str);
    
    // send the server name to the client
    strcat(response, "\r\nServer: ");
    // get the local host name
    char *host_name = (char *)malloc(100*sizeof(char));
    gethostname(host_name, 100);
    printf("Host name: %s\n", host_name);
    strcat(response, host_name);

    return;
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
            printf("Received complete.\n");

            if (ret == -1){
                close(newsockfd);
                exit(0);
            }

            printf("Received request from the client:\n%s\n", request);

            // Message *msg = (Message *)malloc(sizeof(Message));
            Message *msg = init_message();

            char *response = (char *)malloc(MAX_SIZE*sizeof(char));
            int curr_resp_size = MAX_SIZE;

            int res = parse_request(request, msg);
            
            int offset = 0, file_len = 0;
            if (res == -1){
            
                // send the error message to the client
                strcpy(response, "HTTP/1.1 400 Bad Request");

                // add date and server name headers
                add_date_server_name_headers(response);

                // add the standard headers
                add_standard_headers(response);

                // no content to be sent
                strcat(response, "\r\n\r\n");

                offset = strlen(response);
                file_len = 0;

            }

            else if (res == 0){
                // send the response to the client
                if (strcmp(msg->cmd, "GET") == 0){
                    // send the file to the client

                    FILE *fp;
                    file_len = 0;
                    int file_content_len = 0;
                    int file_send_flag = 1;
                    char *file_content_buff = (char *)malloc(MAX_SIZE*sizeof(char));

                    // check the last modified time of the file given in the request
                    struct stat file_stat;
                    
                    if (stat(msg->file_path, &file_stat) == -1){
                        if (errno == ENOENT){
                            printf("File does not exist.\n");
                            // send the 404 error message to the client
                            strcpy(response, "HTTP/1.1 404 Not Found");
                        }

                        else if (errno == EACCES){
                            printf("Permission denied.\n");
                            // send the 403 error message to the client
                            strcpy(response, "HTTP/1.1 403 Forbidden");
                        }

                        else{
                            printf("Error opening the file.\n");
                            // send the 500 error message to the client
                            strcpy(response, "HTTP/1.1 500 Internal Server Error");
                        }
                        file_send_flag = 0; // do not send the file to the client as error has occurred
                    }

                    else{
                        
                        // get the last modified time of the file
                        struct tm *last_mod_time = gmtime(&file_stat.st_mtime);

                        // if the if-modified-since header is present in the request
                        if ( strcmp(msg->if_mod_since, "") != 0){
                            // check if the last modified time of the file is greater than the if-modified-since header
                            struct tm *if_mod_since_time = (struct tm *)malloc(sizeof(struct tm));
                            strptime(msg->if_mod_since, "%a, %d %b %Y %H:%M:%S %Z", if_mod_since_time);

                            if (difftime(mktime(last_mod_time), mktime(if_mod_since_time)) <= 0){
                                // send the 304 error message to the client as the file was modified before the if_mod_since 
                                strcpy(response, "HTTP/1.1 304 Not Modified");
                                file_send_flag = 0; // do not send the file to the client as the file has not been modified
                            }
                        }
                    }

                    if (file_send_flag == 1){
                        // check if the file path exists in the current directory
                        fp = fopen(msg->file_path, "r");

                        if (fp == NULL){
                            if (errno == ENOENT){
                                printf("File does not exist.\n");
                                // send the 404 error message to the client
                                memset(response, 0, curr_resp_size);
                                strcpy(response, "HTTP/1.1 404 Not Found");
                            }

                            else if (errno == EACCES){
                                printf("Permission denied.\n");
                                // send the 403 error message to the client
                                memset(response, 0, curr_resp_size);
                                strcpy(response, "HTTP/1.1 403 Forbidden");
                            }

                            else{
                                printf("Error opening the file.\n");
                                // send the 500 error message to the client
                                memset(response, 0, curr_resp_size);
                                strcpy(response, "HTTP/1.1 500 Internal Server Error");
                            }

                            file_send_flag = 0; // do not send the file to the client as error has occurred

                        }
                    }

                    if (file_send_flag == 1){
                        
                        // start reading the contents of the file in a seperate buffer
                        memset(file_content_buff, 0, MAX_SIZE);
                        int curr_buff_size = MAX_SIZE;
                        file_content_len = 0;

                        int read_bytes = 0;

                        // calculate the length of the file in bytes
                        file_len = 0;
                        fseek(fp, 0, SEEK_END);
                        file_len = ftell(fp);
                        fseek(fp, 0, SEEK_SET);

                        // printf("File length: %d\n", file_len);
                        file_content_buff = (char *)realloc(file_content_buff, file_len*sizeof(char));
                        fread(file_content_buff, sizeof(char), file_len, fp);

                        fclose(fp);

                        // printf("File read successfully.\n");
                        FILE *fp1 = fopen("test.txt", "w");
                        fwrite(file_content_buff, sizeof(char), file_content_len, fp1);

                        // send the response to the client if there is no error in reading the file
                        if (file_send_flag == 1){
                            // send the 200 OK message to the client
                            memset(response, 0, curr_resp_size);
                            strcpy(response, "HTTP/1.1 200 OK");
                        }
                    }

                    // add the date and server headers
                    add_date_server_name_headers(response);

                    // send the last modified time of the file to the client
                    if (file_send_flag == 1){
                        
                        struct stat file_stat;
                        stat(msg->file_path, &file_stat);

                        struct tm *last_mod_time = gmtime(&file_stat.st_mtime);
                        // convert it to local time using localtime()
                        last_mod_time = localtime(&file_stat.st_mtime);

                        char *last_mod_time_str = (char *)malloc(100*sizeof(char));
                        strftime(last_mod_time_str, 100, "%a, %d %b %Y %H:%M:%S %Z", last_mod_time);
                        strcat(response, "\r\nLast-Modified: ");
                        strcat(response, last_mod_time_str);

                        // send the content type of the file to the client
                        char *file_type = (char *)malloc(100*sizeof(char));
                        char *file_ext = strrchr(msg->file_path, '.');

                        if (strcmp(file_ext, ".html") == 0){
                            strcpy(file_type, "text/html");
                        }

                        else if (strcmp(file_ext, ".pdf") == 0){
                            strcpy(file_type, "application/pdf");
                        }

                        else if (strcmp(file_ext, ".jpg") == 0){
                            strcpy(file_type, "image/jpeg");
                        }

                        else 
                            strcpy(file_type, "text/*");

                        strcat(response, "\r\nContent-Type: ");
                        strcat(response, file_type);

                        // send the content length of the file to the client
                        char *file_len_str = (char *)malloc(100*sizeof(char));
                        sprintf(file_len_str, "%d", file_len);
                        strcat(response, "\r\nContent-Length: ");
                        strcat(response, file_len_str);

                        // send the content language of the file to the client
                        char *file_lang = (char *)malloc(100*sizeof(char));
                        strcpy(file_lang, "en-us");
                        strcat(response, "\r\nContent-Language: ");
                        strcat(response, file_lang);
                    }

                    add_standard_headers(response);

                    // add the content of the file to the response if the file_send_flag is set
                    if (file_send_flag == 1){
                        strcat(response, "\r\n\r\n");
                        printf("Sending the response to the client.\n");
                        offset = strlen(response);
                        int request_size = MAX_SIZE;

                        for (int i = 0; i < file_len; i++)
                        {
                            if (offset + i >= request_size)
                            {
                                request_size *= 2;
                                response = realloc(response, request_size * sizeof(char));
                            }
                            response[offset + i] = file_content_buff[i];
                        }

                        // free the memory allocated to the file content buffer
                        free(file_content_buff);
                    }   
                    else{
                            strcat(response, "\r\n\r\n");
                    }
                }

                else if (strcmp(msg->cmd, "PUT") == 0){
                    //
                }

            }

            printf("Sending the response to the client.\n");
            printf("%s\n", response);

            int bytes_sent = send(newsockfd, response, offset + file_len , 0);

            if (bytes_sent < 0){
                printf("Error sending the response to the client.\n");
                exit(1);
            }

            // parsing the http request to get the command, url, host, port
            printf("Closing the connection with the client.\n");
            close(newsockfd);
            exit(0);
            
        }

        close(newsockfd);
    
    }

    close(sockfd);
    return 0;

}