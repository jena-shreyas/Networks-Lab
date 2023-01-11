#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHUNK_SIZE 10

int main(){

    char* buf;
    buf = (char *)malloc((CHUNK_SIZE + 1) * sizeof(char));
    // scanf("%10s", buf);
    fgets(buf, 10 + 1, stdin);
    printf("%s\n", buf);

    if (!strcmp(buf, "-1")){
        printf("Same");
    }
    else    printf("Not same!");

    fgets(buf, 10+1, stdin);
    printf("%s", buf);

    return 0;
}