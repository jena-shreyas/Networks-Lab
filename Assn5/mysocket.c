#include "mysocket.h"

// function to handle SIGINT
void sigint_handler(int signum)
{
    printf("SIGINT received!\n");
    if (my_close(mysocket.sockfd) < 0) {
        perror("Unable to close socket!\n");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

void* send_message(void* sockfd){
    printf("Sending message ...\n");
    return 0;
}

void* recv_message(void* sockfd){
    printf("Receiving message ...\n");
    return 0;
}

int my_socket(int domain, int type, int protocol)
{
    // register SIGINT handler
    signal(SIGINT, sigint_handler);
    memset(&mysocket, 0, sizeof(mysocket));
    if (type == SOCK_MyTCP){

        if ((mysocket.sockfd = socket(domain, SOCK_STREAM, protocol)) < 0) {
            perror("Cannot create the socket!\n");
            return mysocket.sockfd;
        }   

        // create threads
        printf("Creating threads ...\n");
        pthread_create(&mysocket.R, NULL, recv_message, &(mysocket.sockfd));
        pthread_create(&mysocket.S, NULL, send_message, &(mysocket.sockfd));

        // initialize and allocate memory for send and receive buffers
        printf("Initializing buffers ...\n");
        int num_messages = MAX_MESSAGE_SIZE/MAX_SEND_SIZE;
        mysocket.Send_Message = (char **)malloc(num_messages * sizeof(char *));
        mysocket.Received_Message = (char **)malloc(num_messages * sizeof(char *));

        for (int i = 0; i < num_messages; i++) {
            mysocket.Send_Message[i] = (char *)malloc(MAX_SEND_SIZE * sizeof(char));
            mysocket.Received_Message[i] = (char *)malloc(MAX_SEND_SIZE * sizeof(char));
        }

        return mysocket.sockfd;
    }

    else {
        perror("Invalid socket type!\n");
        return -1;
    }
}

int my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    int bind_status;
    if ((bind_status = bind(sockfd, addr, addrlen)) < 0) {
        perror("Unable to bind local address!\n");
    }
    return bind_status;
}

int my_listen(int sockfd, int backlog)
{
    int listen_status;
    if ((listen_status = listen(sockfd, backlog)) < 0) {
        perror("Unable to listen on socket!\n");
    }
    return listen_status;
}

int my_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int accept_status;
    if ((accept_status = accept(sockfd, addr, addrlen)) < 0) {
        perror("Unable to accept connection!\n");
    }
    return accept_status;
}

int my_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    int connect_status;
    if ((connect_status = connect(sockfd, addr, addrlen)) < 0) {
        perror("Unable to connect to remote host!\n");
    }
    return connect_status;
}

int my_send(int sockfd, const void *buf, size_t len, int flags)
{

}

int my_recv(int sockfd, void *buf, size_t len, int flags)
{

}

int my_close(int sockfd)
{
    int close_status;
    if ((close_status = close(sockfd)) < 0) {
        perror("Unable to close socket!\n");
    }

    /*
        clear contents of mysocket
        this also deletes the sockfd stored in it
        done because at the end, this fd will be the last one to refer to the actual socket
        so it should be closed
    */

    // join threads
    printf("Joining threads ...\n");
    pthread_join(mysocket.R, NULL);
    pthread_join(mysocket.S, NULL);

    // free buffers
    printf("Freeing buffers ...\n");
    int num_messages = MAX_MESSAGE_SIZE/MAX_SEND_SIZE;
    for (int i = 0; i < num_messages; i++) {
        free(mysocket.Send_Message[i]);
        free(mysocket.Received_Message[i]);
    }
    free(mysocket.Send_Message);
    free(mysocket.Received_Message);

    // clear mysocket
    printf("Clearing mysocket ...\n");
    memset(&mysocket, 0, sizeof(mysocket));         
    return close_status;
}
