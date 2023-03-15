#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

#define MAX_SEND_SIZE 1000
#define MAX_MESSAGE_SIZE 5000
#define SOCK_MyTCP SOCK_STREAM
#define MAX_TABLE_SIZE 10

typedef struct _message{
    size_t length;
    char *data;
    int flag;
} message;

typedef struct _data_table{
    int num_entries;
    message *messages;
    int num_full;
} data_table;

typedef struct myTCP{

    int sockfd;
    pthread_t R;
    pthread_t S;
    data_table *send_message;
    data_table *recv_message;
}MyTCP;


int my_socket(int domain, int type, int protocol);
int my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int my_listen(int sockfd, int backlog);
int my_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int my_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int my_close(int sockfd);
int my_send(int sockfd, const void *buf, size_t len, int flags);
int my_recv(int sockfd, void *buf, size_t len, int flags);

void* send_message(void* sockfd);
void* recv_message(void* sockfd);