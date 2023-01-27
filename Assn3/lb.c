#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <time.h>
#define BUF_SIZE 50
#define MAX_SIZE 100
#define TIMEOUT 5
#define NUM_SERVERS 2

int main(int args, char* argv[]){

    int servsockfd, clisockfd, newsockfd;   // define 1 clisockfd for client connections, 1 servsockfd for server connections
    socklen_t clilen;
    int loads[NUM_SERVERS];                 // array to store load of each server
    time_t start_time, curr_time, elapsed;
    struct sockaddr_in serv1addr, serv2addr, lbaddr, cliaddr;
    char buf[BUF_SIZE];
    char tmp[MAX_SIZE];

    lbaddr.sin_family = AF_INET;
    lbaddr.sin_addr.s_addr = INADDR_ANY;
    lbaddr.sin_port = htons(atoi(argv[1]));

    serv1addr.sin_family = AF_INET;
    serv1addr.sin_port = htons(atoi(argv[2]));
    inet_aton("127.0.0.1", &serv1addr.sin_addr);

    serv2addr.sin_family = AF_INET;
    serv2addr.sin_port = htons(atoi(argv[3]));
    inet_aton("127.0.0.1", &serv2addr.sin_addr);

    loads[0] = rand() % 100 + 1;       // generate random load for server 1 initially
    loads[1] = rand() % 100 + 1;       // generate random load for server 2 initially

    // Set up LB clisockfd to act as server for the clients
    if ((clisockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("LB socket could not be created!");
        exit(EXIT_FAILURE);
    }

    // Bind clisockfd to local address
    if ((bind(clisockfd, (struct sockaddr*)&lbaddr, sizeof(lbaddr))) < 0){

        perror("Could not bind local address to client LB socket!");
        exit(EXIT_FAILURE);
    }

    listen(clisockfd, 2);               // set limit for concurrent client connections

    while (1) {

        struct pollfd fdset[1];             // defines set of sockfds monitored by poll()
        fdset[0].fd = clisockfd;            // set 1st poll sockfd to our UDP socket
        fdset[0].events = POLLIN;           // define "normal read without blocking" (POLLIN) as event to be monitored

        time(&start_time);                  // store start time
        curr_time = start_time;             // store current time (initially same as start time)
        elapsed = curr_time - start_time;   // store difference between current and start time

        while (elapsed < TIMEOUT){          // while time elapsed is less than timeout

            printf("Waiting for %ld seconds for client connection request ...\n", (TIMEOUT - elapsed));
            int ret = poll(fdset, 1, (TIMEOUT - elapsed)*1000);      // wait on poll() for the remaining time

            // if poll detects a client connection request
            if (ret > 0){    

                // use single sockfd (clisockfd) to handle all client requests
                clilen = sizeof(cliaddr);
                newsockfd = accept(clisockfd, (struct sockaddr*)&cliaddr, &clilen);

                if (newsockfd < 0){
                    perror("Error while connecting to LB");
                    exit(EXIT_FAILURE);
                }

                if (fork() == 0){
                    
                    // Set up servsockfd to act as client for the servers
                    if ((servsockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
                        perror("LB socket could not be created!");
                        exit(EXIT_FAILURE);
                    }

                    printf("Server 1 load: %d\n", loads[0]);
                    printf("Server 2 load: %d\n", loads[1]);

                    // collect time from server 1 or server 2, whichever has less load
                    if (loads[0] < loads[1]){

                        if (connect(servsockfd, (struct sockaddr*)&serv1addr, sizeof(serv1addr)) < 0){
                            perror("LB could not be connected to server 1");
                            exit(EXIT_FAILURE);
                        }
                        else    
                            printf("Sending client request to server 1 : %s\n", inet_ntoa(serv1addr.sin_addr));
                    }
                    else{

                        if (connect(servsockfd, (struct sockaddr*)&serv2addr, sizeof(serv2addr)) < 0){
                            perror("LB could not be connected to server 2");
                            exit(EXIT_FAILURE);
                        }
                        else    
                            printf("Sending client request to server 2 : %s\n", inet_ntoa(serv2addr.sin_addr));
                    }

                    memset(buf, '\0', BUF_SIZE);
                    strcpy(buf, "Send Time");

                    // Send request to server 1 or server 2
                    send(servsockfd, buf, strlen(buf) + 1, 0);

                    memset(buf, '\0', BUF_SIZE);
                    memset(tmp, '\0', MAX_SIZE);

                    // receive time from server 1 or server 2 in chunks
                    while (1){
                            
                        memset(buf, '\0', BUF_SIZE);
                        recv(servsockfd, buf, BUF_SIZE, 0);
                        strcat(tmp, buf);
        
                        if (strlen(buf) < (BUF_SIZE - 1))
                            break;
                    }

                    // once time is received, close connection with server 1 or server 2
                    close(servsockfd);

                    // send time to client
                    printf("Time from server : %s\n", tmp);
                    send(newsockfd, tmp, strlen(tmp) + 1, 0);
                    close(newsockfd);
                    exit(0);
                }

                else
                    close(newsockfd);
            }

            time(&curr_time);                       // update current time
            elapsed = curr_time - start_time;       // update difference between current and start time
        }

        // Once timeout occurs, check load of servers
        // Set up LB socket to act as client for the servers

        // Create socket and collect load from server 1
        if ((servsockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            perror("LB socket could not be created!");
            exit(0);
        }

        if (connect(servsockfd, (struct sockaddr*)&serv1addr, sizeof(serv1addr)) < 0){
            perror("LB could not be connected to server 1");
            exit(EXIT_FAILURE);
        }

        memset(buf, '\0', BUF_SIZE);
        memset(tmp, '\0', MAX_SIZE);

        strcpy(buf, "Send Load");
        send(servsockfd, buf, strlen(buf) + 1, 0);

        // receive server 1 load in chunks
        while (1){

            memset(buf, '\0', BUF_SIZE);
            recv(servsockfd, buf, BUF_SIZE, 0);
            strcat(tmp, buf);

            if (strlen(buf) < (BUF_SIZE - 1))
                break;
        }

        loads[0] = atoi(tmp);              // store load of server 1
        printf("Load received from server 1 (%s) : %d\n", inet_ntoa(serv1addr.sin_addr), loads[0]);
        close(servsockfd);


        // Create socket and collect load from server 2
        if ((servsockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            perror("LB socket could not be created!");
            exit(0);
        }

        if (connect(servsockfd, (struct sockaddr*)&serv2addr, sizeof(serv2addr)) < 0){
            perror("LB could not be connected to server 2");
            exit(EXIT_FAILURE);
        }

        memset(buf, '\0', BUF_SIZE);
        memset(tmp, '\0', MAX_SIZE);

        strcpy(buf, "Send Load");
        send(servsockfd, buf, strlen(buf) + 1, 0);

        // receive server 2 load in chunks
        while (1){

            memset(buf, '\0', BUF_SIZE);
            recv(servsockfd, buf, BUF_SIZE, 0);
            strcat(tmp, buf);

            if (strlen(buf) < (BUF_SIZE - 1))
                break;
        }

        loads[1] = atoi(tmp);              // store load of server 2
        printf("Load received from server 2 (%s) : %d\n", inet_ntoa(serv2addr.sin_addr), loads[1]);
        close(servsockfd);

    }
    return 0;
}