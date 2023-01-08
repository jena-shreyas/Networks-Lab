#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(){
    
    int sockfd;
    struct sockaddr_in serv_addr;
    char ch;
    size_t len = 0, size = 0;
    char* buf = NULL;

    buf = realloc(NULL, sizeof(char) * size);
    printf("Enter a valid arithmetic expression : ");
    
    while ((ch = getchar()) != '\n'){

        if (len + 1 >= size){

            size = 2 * size + 1;
            buf = realloc(buf, sizeof(char) * size);
        }

        buf[len++] = ch;
    }
		
    printf("Length : %zu", len);
    printf("Expression : %s", buf);
	return 0;

}