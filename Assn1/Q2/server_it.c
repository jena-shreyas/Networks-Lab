#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#define CHUNK_SIZE 5
#define RESULT_SIZE 40
#define PRECISION 6

double* evaluate(char* buf){
	
	char ch;
	int i = 0;
	double number = 0, result = 0;
	double *ptr = NULL;

	// code to evaluate infix expression from left-to-right order without any precedence
	// allowing a single pair of brackets and without using any stack
	while ((ch = buf[i++]) != '\0'){

		if (ch == ' ')	continue;

		else if (ch >= '0' && ch <= '9'){

			result = result * 10.0 + (ch - '0');
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

		else if (ch == '('){

			int brack_len = 0;
			char* brack_start = buf + i * sizeof(char);
			printf("Starting char : %c\n", *brack_start);

			while ((ch = buf[i++]) != ')')
				brack_len++;

			char* exp = (char *)malloc((brack_len + 1) * sizeof(char));
			strncpy(exp, brack_start, brack_len);
			exp[brack_len] = '\0';
			printf("Exp : %s\n", exp);

			double* brack_result = evaluate(exp);
			number = *brack_result;
			result += number;
		}

		else if (ch == '+' || ch == '-' || ch == '*' || ch == '/'){

			number = 0;
			char op = ch; 	// used to store the operator
			while (1){

				ch = buf[i++];

				if (ch == ' ')	
					continue;

				else if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '\0')
					break;

				else if (ch >= '0' && ch <= '9'){

					number = number * 10.0 + (ch - '0');
					// printf("Number : %lf", number);
				}

				else if (ch == '.'){

					double decimal = 10.0;
					while (1){

						ch = buf[i++];
						if (ch == ' ')	continue;
						else if (ch >= '0' && ch <= '9'){

							number += (ch - '0') / decimal;
							// printf("Number : %lf", number);
							decimal *= 10;
						}
						else {
							i--;
							break;
						}
					}
				}
			}

			if (op == '+')		result += number;
			else if (op == '-')	result -= number;
			else if (op == '*')	result *= number;
			else if (op == '/')	{
				
				if (number > -pow(10, -PRECISION) && number < pow(10, -PRECISION))		// specifying the precision for numbers considered as zero
					return NULL;
				else
					result /= number;
			}

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

	ptr = &result;
	return ptr;
}

int main()
{
	int	sockfd, newsockfd; 
	int	clilen;
	int size;
	char *buf, *chunk_buf;
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

	// loop for handling multiple users
	while (1) {		

		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

		if (newsockfd < 0) {
			perror("Error while accepting connection request!\n");
			exit(0);
		}

		chunk_buf = (char *)malloc(CHUNK_SIZE * sizeof(char));

		// loop for handling multiple requests from user
		while (1){		

			buf = NULL;
			size = 0;

			// loop for chunk-wise handling of client input
			while(1){	

				int bytes_recv = recv(newsockfd, chunk_buf, CHUNK_SIZE, 0);				
				// printf("Received : %s\n", chunk_buf);

				size = size + bytes_recv;
				// printf("Reallocating ...\n");
				buf = realloc(buf, size);
				// printf("Concatenate ...\n");
				strcat(buf, chunk_buf);
				
				char* null_ptr = strchr(chunk_buf, '\0');
				size_t null_idx = (null_ptr - chunk_buf)/ sizeof(char); 
				
				// if null character detected at non-end position, end of input reached
				if (null_idx < CHUNK_SIZE){		

					// printf("End of input reached!\n");
					break;
				}				
			}

			if (!strcmp(buf, "-1"))	{
					
				printf("Client closed connection.\n\n");
				break;
			}

			printf("Expression : %s\n", buf);
			double *ans = evaluate(buf);

			if (ans == NULL)
				strcpy(result, "Error : Division by zero!");
			else
				sprintf(result, "%lf", *ans);			    // compute result and store it in result

			send(newsockfd, result, RESULT_SIZE, 0);		// send result to client
			free(buf);
		}

		close(newsockfd);
		free(chunk_buf);
	}
	return 0;
}
			

