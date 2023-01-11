#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#define CHUNK_SIZE 10
#define RESULT_SIZE 20

double evaluate(char* buf){
	
	char ch;
	int i = 0, number = 0;
	double result = 0;

	// code to evaluate infix expression from left-to-right order without any precedence
	// allowing a single pair of brackets and without using any stack
	while ((ch = buf[i++]) != '\0'){

		if (ch == ' ')	continue;

		else if (ch >= '0' && ch <= '9'){

			number = number * 10 + (ch - '0');
			result = number;
		}

		else if (ch == '.'){

			double decimal = 10.0;
			while (1){

				ch = buf[i++];
				if (ch == ' ')	continue;
				if (ch >= '0' && ch <= '9'){

					result += (ch - '0') / decimal;
					decimal *= 10;
				}
				else break;
			}
			i--;
		}
		else if (ch == '+' || ch == '-' || ch == '*' || ch == '/'){

			number = 0;
			char op = ch; 	// used to store the operator
			while (1){

				ch = buf[i++];
				if (ch == ' ')	continue;
				if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '\0')		break;

				number = number * 10 + (ch - '0');
			}

			if (op == '+')		result += number;
			else if (op == '-')	result -= number;
			else if (op == '*')	result *= number;
			else if (op == '/')	result /= number;
			i--;
		}

		// if (ch == '('){

		// 	number = 0;
		// 	while (1){

		// 		ch = buf[i++];
		// 		if (ch == ')')		break;

		// 		number = number * 10 + (ch - '0');
		// 	}
		// 	result += number;
		// }

	}

	return result;
}

int main()
{
	int	sockfd, newsockfd; 
	int	clilen;
	int size;
	char *buf, *chunk_buf, ch;
	char result[RESULT_SIZE];
	struct sockaddr_in	cli_addr, serv_addr;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(20000);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("Unable to bind local address\n");
		exit(0);
	}

	listen(sockfd, 5); 
	while (1) {		// loop for handling multiple users

		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

		if (newsockfd < 0) {
			perror("Error while accepting connection request!\n");
			exit(0);
		}

		chunk_buf = (char *)malloc((CHUNK_SIZE + 1) * sizeof(char));

		while (1){		// loop for handling multiple requests from user

			buf = NULL;
			size = 0;

			while(1){

				int bytes_recv = recv(newsockfd, chunk_buf, CHUNK_SIZE, 0);				if (!strcmp(buf, "-1"))	{
					
					printf("Client closed connection.\n\n");
					break;
				}
				size = size + bytes_recv;
				buf = realloc(buf, size);
				strncat(buf, chunk_buf, bytes_recv);

				if (strchr(chunk_buf, '\0') != NULL)	
					break;
				
			}
			// while (1){	// loop for char-wise handling of client input

			// 	if (len + 1 >= size){

			// 		size = 2 * size + 1;
			// 		buf = realloc(buf, size);
			// 	}

			// 	recv(newsockfd, &ch, 1, 0);
			// 	if (ch == '\0')		break;

			// 	// printf("Received : %c\n", ch);
			// 	buf[len++] = ch;
			// }

			if (!strcmp(buf, "-1")){

				printf("Client closed connection.\n\n");
				break;
			}

			printf("Length : %d\n", strlen(buf));
			printf("Expression : %s\n", buf);

			sprintf(result, "%lf", evaluate(buf));			// compute result and store it in result
			send(newsockfd, result, RESULT_SIZE, 0);		// send result to client

			free(buf);
		}

		close(newsockfd);
		free(chunk_buf);
	}
	return 0;
}
			

