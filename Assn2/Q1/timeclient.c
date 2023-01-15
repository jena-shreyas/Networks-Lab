#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <poll.h>
#define MAX_SIZE 100
#define MAX_TRIES 5
#define TIMEOUT 3000

int main(){

    int sockfd;
    struct sockaddr_in servaddr;
    int i, num_tries;
    char buf[MAX_SIZE];
    socklen_t len;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0){
        perror("Client socket creation error\n");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family =  AF_INET;
    servaddr.sin_port = htons(8181);
    inet_aton("127.0.0.1", &servaddr.sin_addr);

    for (i=0; i<MAX_SIZE; i++)   buf[i] = '\0';

    len = sizeof(servaddr);
    num_tries = 0;                  // count no. of tries 

    struct pollfd fdset[1];         // defines set of sockfds monitored by poll()
    fdset[0].fd = sockfd;           // set 1st poll sockfd to our UDP socket
    fdset[0].events = POLLIN;       // define "normal read without blocking" (POLLIN) as event to be monitored

    // run till max tries limit (5) exceeded
    while (num_tries < MAX_TRIES){

        printf("Try : %d\n", (num_tries+1));

        // first send a message to server, so server knows this client's address
        strcpy(buf, "Message from client");
        sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));

        // set up poll) for a timeout of 3 seconds
        int ret = poll(fdset, 1, TIMEOUT);
        printf("poll() returned : %d\n", ret);

        // if poll detects the expected event in the socket
        if (ret > 0){       
            if (fdset[0].revents == POLLIN){

                //if read data without blocking possible, receive time from server, close client
                int n = recvfrom(sockfd, buf, MAX_SIZE, 0, (struct sockaddr*)&servaddr, &len);
                buf[n] = '\0';
                printf("Server local time : %s", buf);
                close(sockfd);
                exit(EXIT_SUCCESS);
            }
        }
        num_tries++;
    }

    printf("Number of tries : %d\n", num_tries);

    // if max tries exceeded
    if (num_tries == MAX_TRIES){        

        // print timeoue exceeded message and close client
        printf("Timeout exceeded\n");   
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return 0;
}