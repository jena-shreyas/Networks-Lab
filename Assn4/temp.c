#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <URL>\n", argv[0]);
        return 1;
    }

    char *hostname = argv[1];
    char *port = "80";
    char *protocol = "http";
    char *path = "/";
    char *temp = (char *)malloc(strlen(hostname) + 1);

    // Find hostname and path in the URL
    char *p = strstr(hostname, "://");
    if (p != NULL) {
        protocol = strtok(hostname, "://");
        hostname = p + 3;
    }
    printf("Hostname: %s\n", hostname);

    p = strchr(hostname, '/');
    if (p != NULL) {
        path = p;
        *p = '\0';
    }
    // printf("Path: %s\n", path);
    // printf("Hostname: %s\n", hostname);
    p = strchr(hostname, ':');
    if (p != NULL) {
        *p = '\0';
        port = p + 1;
        // temp = p + 1;
    }
    // printf("Port no: %s\n", temp);

    printf("hostname: %s\n", hostname);
    printf("port: %s\n", port);

    //Get the IP address of the host
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int status = getaddrinfo(hostname, port, &hints, &res);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    }

    // Extract the IP address and port number
    struct sockaddr_in *addr = (struct sockaddr_in *) res->ai_addr;
    char ip_address[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr->sin_addr, ip_address, sizeof ip_address);
    int port_number = ntohs(addr->sin_port);

    // Print the results
    printf("Protocol: %s\n", protocol);
    printf("Hostname: %s\n", hostname);
    printf("Path: %s\n", path);
    printf("IP address: %s\n", ip_address);
    printf("Port number: %d\n", port_number);

    //Clean up
    freeaddrinfo(res);
    return 0;
}
