#include<pthread.h>
#include<stdio.h>

int *addr;

void *thread(void *ptr){
    int type = (int )ptr;
    if (type == 1){
        int a = 10; // local variable
        addr = &a;
    }
    while(1){
        if (*addr == 10){
            *addr = 20;
        }
        else if (*addr == 20){
            *addr = 10;
        }
        printf("%d %d\n", type, *addr);
    }
    return ptr;
}

int main(){
    pthread_t thread1, thread2;
    int thr = 1, thr2 = 2;
    pthread_create(&thread1, NULL, *thread, (void *)thr);
    pthread_create(&thread2, NULL, *thread, (void *)thr2);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    return 0;
}