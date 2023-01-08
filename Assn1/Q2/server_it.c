/*
			NETWORK PROGRAMMING WITH SOCKETS

In this program we illustrate the use of Berkeley sockets for interprocess
communication across the network. We show the communication between a server
process and a client process.


*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>


int main()
{
	int	sockfd, newsockfd; 
	int	clilen;
	size_t len, size;
	char *buf, ch;
	struct sockaddr_in	cli_addr, serv_addr;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(20000);

	if (bind(sockfd, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		perror("Unable to bind local address\n");
		exit(0);
	}

	listen(sockfd, 5); 
	while (1) {

		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

		if (newsockfd < 0) {
			perror("Accept error\n");
			exit(0);
		}

		buf = NULL;
		len = 0, size = 0;

		while (1){

			if (len + 1 >= size){

				size = 2 * size + 1;
				buf = realloc(buf, size);
			}

			recv(newsockfd, &ch, 1, 0);
			if (ch == '\0')		break;

			printf("Received : %c\n", ch);
			buf[len++] = ch;
		}

		printf("Length : %zu\n", len);
		printf("Expression : %s\n", buf);

		close(newsockfd);
	}
	return 0;
}
			

