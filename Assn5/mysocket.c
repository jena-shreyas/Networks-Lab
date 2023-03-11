#include "mysocket.h"

int my_socket(int domain, int type, int protocol)
{
    int sockfd;
    if (type == SOCK_MyTCP){

        if ((sockfd = socket(domain, SOCK_STREAM, protocol)) < 0) {
            perror("Cannot create the socket!\n");
        }   
        return sockfd;
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
    return close_status;
}
