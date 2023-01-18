#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#define CMD_SIZE 10
#define BUF_SIZE 50
#define MAX_SIZE 200

int main(){

    int sockfd, newsockfd;
    int clilen;
    int i;
    struct sockaddr_in servaddr, cliaddr;
    char buf[BUF_SIZE];
    char str[MAX_SIZE];
    char *line = NULL;
    short found;

    FILE *fp;
    const char *filename = "users.txt";
    size_t len = 0;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0){
        printf("Server socket creation error\n");
        exit(EXIT_FAILURE);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;    
    servaddr.sin_port = htons(20000);

    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
        printf("Could not bind local address\n");
        exit(EXIT_FAILURE);
    }

    listen(sockfd, 5);

    while (1){

        clilen = sizeof(cliaddr);

        newsockfd = accept(sockfd, (struct sockaddr*)&cliaddr, &clilen);

        if (newsockfd < 0){
            printf("Error while accepting connection request\n");
            exit(EXIT_FAILURE);
        }

        if (fork() == 0){

            close(sockfd);

            for (i=0;i<BUF_SIZE;i++)    buf[i] = '\0';

            // send login prompt to client
            strcpy(buf, "LOGIN:");
            send(newsockfd, buf, strlen(buf) + 1, 0);

            // receive username
            recv(newsockfd, buf, BUF_SIZE, 0);
            printf("Username : %s\n", buf);

            fp = fopen(filename, "r");

            found = 0;

            // open users.txt and check username, send status
            while (getline(&line, &len, fp) != -1){

                // remove trailing newline in input line, if any
                size_t ln = strlen(line) - 1;
                if (line[ln] == '\n')
                    line[ln] = '\0';

                if (!strcmp(buf, line)){

                    strcpy(buf, "FOUND");
                    printf("User found!\n\n");
                    send(newsockfd, buf, strlen(buf) + 1, 0);    // send username status
                    found = 1;
                    break;
                }
            }

            fclose(fp);

            if (found == 0){

                strcpy(buf, "NOT-FOUND");
                printf("Invalid username\n\n");
                send(newsockfd, buf, strlen(buf) + 1, 0);           // send username status
                close(newsockfd);
                exit(0);

            }

            else {

                // handle multiple inputs for user
                while(1) {

                    char* cmd;      // stores shell command
                    char* dir;      // stores input directory
                    char* ptr;

                    cmd = (char *)malloc(CMD_SIZE * sizeof(char));
                    dir = (char *)malloc(MAX_SIZE * sizeof(char));

                    // clearing str for storing input
                    for (int j = 0; j < MAX_SIZE;j++)    str[j]='\0';

                    // Input chunk receiving and assembling
                    while (1){

                        recv(newsockfd, buf, BUF_SIZE, 0);
                        strcat(str, buf);

                        if (strlen(buf) < (BUF_SIZE - 1))
                            break;
                    }

                    // Split input into shell command + directory
                    ptr = strtok(str, " ");
                    strcpy(cmd, ptr);       // store command

                    if (ptr != NULL){

                        ptr = strtok(NULL, " ");
                        if (ptr != NULL)
                            strcpy(dir, ptr);      // store directory path
                        
                    }

                    // clearing str for storing result
                    for (int j = 0; j < MAX_SIZE;j++)    str[j]='\0';

                    // if command = exit
                    if (!strcmp(cmd, "exit")){

                        // printf("Exiting ...\n");
                        close(newsockfd);
                        exit(0);
                    }

                    // if command = pwd
                    else if (!strcmp(cmd, "pwd")){

                        char cwd[MAX_SIZE];

                        // error in running pwd
                        if (getcwd(cwd, sizeof(cwd)) == NULL){

                            strcpy(buf, "####");
                            send(newsockfd, buf, strlen(buf) + 1, 0);
                            continue;
                        }

                        else
                            // copy getcwd output to str
                            strcpy(str, cwd);       
                                
                    }

                    // if command = dir
                    else if (!strcmp(cmd, "dir")){

                        DIR *pDir;

                        // No argument passed, choose current directory
                        if (strlen(dir) == 0)     
                            strcpy(dir, ".");

                        pDir = opendir(dir);
                            
                        if (pDir == NULL){

                            strcpy(buf, "####");
                            send(newsockfd, buf, strlen(buf) + 1, 0);
                            continue;
                        }

                        else{

                            struct dirent *dent;

                            while((dent = readdir(pDir)) != NULL){

                                strcat(str, dent->d_name);
                                strcat(str, "\n");
                            }

                            closedir(pDir);
                        }
                    }

                    // if command = cd
                    else if (!strcmp(cmd, "cd")){

                        char cwd[MAX_SIZE];

                        // No argument passed, choose current directory
                        if (strlen(dir) == 0)     
                            strcpy(dir, ".");

                        if (!chdir(dir)){       // chdir returns 0 when successful

                            getcwd(cwd, sizeof(cwd));
                            strcpy(str, cwd);
                        }

                        else{                   // chdir returns -1 when unsuccessful

                            strcpy(buf, "####");
                            send(newsockfd, buf, strlen(buf) + 1, 0);
                            continue;
                        }
                    }

                    // if invalid command
                    else {

                        strcpy(buf, "$$$$");
                        send(newsockfd, buf, strlen(buf) + 1, 0);
                        continue;
                    }

                    int i = 0, num_chars = 0;

                    // clearing buffer
                    for (int j = 0; j<BUF_SIZE; j++)    buf[j] = '\0';

                    // sending input in buffer-sized chunks
                    while (str[i] != '\0'){

                        buf[num_chars++] = str[i++];

                        if (i % (BUF_SIZE - 1) == 0){

                            send(newsockfd, buf, strlen(buf) + 1, 0);

                            for (int j = 0; j < BUF_SIZE; j++)    buf[j] = '\0';      // clear buffer
                            num_chars = 0;

                        }
                    }

                    // send the last packet
                    send(newsockfd, buf, strlen(buf) + 1, 0);

                    free(cmd);
                    free(dir);
                }
            }
        }

        close(newsockfd);
    }
    return 0;
}