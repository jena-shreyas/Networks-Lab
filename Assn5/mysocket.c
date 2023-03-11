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

