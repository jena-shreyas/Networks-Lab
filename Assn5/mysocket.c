#include "mysocket.h"
#include <string.h>

MyTCP mysocket;

pthread_mutex_t send_table_mutex;
pthread_mutex_t recv_table_mutex;
pthread_mutex_t connect_flag_mutex;
pthread_mutex_t stdout_mutex;

int connect_flag = -1;
int newfd = -1;

// function to handle SIGINT
void sigint_handler(int signum)
{
    printf("\nSIGINT received!\n");
    if (my_close(mysocket.sockfd) < 0) {
        perror("Unable to close socket!\n");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

int min_val(int a, int b){
    if (a < b) return a;
    return b;
}

int send_in_chunks(int sock_fd,  char* data, size_t length){
    int bytes_sent = 0;
    int total_bytes = 0;

    char *str = (char*)malloc((length + 4)*sizeof(char));
    memset(str, 0, length + 4);
    // first 4 bytes of string is the message length

    // str[0] = (length >> 24) & 0xFF;
    // str[1] = (length >> 16) & 0xFF;
    // str[2] = (length >> 8) & 0xFF;
    // str[3] = length & 0xFF;
    // printf("str[0] = %d\n", str[0]);
    // printf("str[1] = %d\n", str[1]);
    // printf("str[2] = %d\n", str[2]);
    // printf("str[3] = %d\n", str[3]);

    // convert size_t to char*
    char* temp = (char*)malloc(5*sizeof(char));
    memset(temp, 0, 5);
    snprintf(temp, 5, "%04d", (int)length);
    // printf("temp = %s\n", temp);
    for (int i = 0; i < 4; i++){
        // printf("temp[%d] = %d\n", i, temp[i]);
        str[i] = temp[i];
    }
    // printf("temp[4] = %d\n", temp[4]);

    // pthread_mutex_lock(&stdout_mutex);
    // printf(" [Send Thread] Length of message = %ld\n", length);
    // pthread_mutex_unlock(&stdout_mutex);
    // strcat(str, (char*)length);
    // printf("Hi\n");
    for (int i = 0; i < length; i++){
        // printf("data[%d] = %c\n", i, data[i]);
        str[i+4] = data[i];
    }
    

    // send the message in chunks of MAX_SEND_SIZE bytes till the full message is sent
    while (total_bytes < length + 4){
        // printf("send in chunks: total_bytes = %d\n", total_bytes);
        if ( (bytes_sent = send(sock_fd, str + total_bytes, min_val(MAX_SEND_SIZE, length + 4 - total_bytes), 0) ) != min_val(MAX_SEND_SIZE, length + 4 - total_bytes)){
            // printf("send in chunks: bytes_sent = %d\n", bytes_sent);
            perror("Unable to send message!\n");
            return -1;
        }
        
        // printf("Send in chunks: bytes_sent = %d\n", bytes_sent);
        if (bytes_sent < 0){
            // perror("Unable to send message!\n");
            return -1;
        }
        total_bytes += bytes_sent;
    }
    free(str);
    free(temp);
    // pthread_mutex_lock(&stdout_mutex);
    // printf("[Send Thread] Sent a message of size %d bytes!\n", total_bytes);
    // pthread_mutex_unlock(&stdout_mutex);
    return total_bytes - 4;
}

void* send_message(void* sockfd){

    int send_sock = *((int*)sockfd);
    printf("[Send Thread] Thread S has been created!\n");

    pthread_mutex_lock(&connect_flag_mutex);
    int it = 0;
    while(connect_flag == -1){
        pthread_mutex_unlock(&connect_flag_mutex);
        if (it == 0){
            printf("[ Send Thread ]: Waiting for connection to be established ...\n");
            it = 1;
        }
        
        sleep(1);
        pthread_mutex_lock(&connect_flag_mutex);
    }
    pthread_mutex_unlock(&connect_flag_mutex);

    if (newfd != -1){
        send_sock = newfd;
        printf("[ Send Thread ]: New socket created for sending messages!\n");
    }

    while(1){

        // printf("[Send Thread] Thread S going to sleep ...\n");
        sleep(1);
        // printf("[Send Thread] Thread S woke up!\n");

        pthread_mutex_lock(&send_table_mutex);
        if (mysocket.send_message->num_full == 0){
            pthread_mutex_unlock(&send_table_mutex);
            // printf("[Send Thread]: No new message in the Send Table to be sent!\n");
            continue;
        }
        else{
            // pthread_mutex_unlock(&send_table_mutex);
            // printf("[Send Thread] Sending pending %d messages ...\n", mysocket.send_message->num_full);

            for (int i = 0; i < MAX_TABLE_SIZE; i++){
                
                // pthread_mutex_lock(&send_table_mutex);
                if (mysocket.send_message->messages[i].flag == 1){
                    printf("[Send Thread] Sending message at index %d ...\n", i);

                    // NOTE : SEND HAS TO BE MADE IN CHUNKS TILL FULL MESSAGE IS SENT
                    int bytes_sent = send_in_chunks(send_sock, mysocket.send_message->messages[i].data, mysocket.send_message->messages[i].length);
                    printf("[ Send Thread ]: Bytes sent = %d\n", bytes_sent);
                    if (bytes_sent < 0){
                        perror("[Send Thread] Unable to send message!\n");
                        exit(EXIT_FAILURE);
                    }

                    if (bytes_sent != mysocket.send_message->messages[i].length){
                        perror("[Send Thread] Unable to send full message!\n");
                        exit(EXIT_FAILURE);
                    }
                    else{
                        printf("[Send thread] Sent a message of size %d bytes at index %d!\n", bytes_sent, i);
                    }

                    mysocket.send_message->messages[i].flag = 0;
                    mysocket.send_message->num_full--;
                }
                // pthread_mutex_unlock(&send_table_mutex);

            }
            pthread_mutex_unlock(&send_table_mutex);
        }

    }

}

int my_send(int sockfd, const void *buf, size_t len, int flags)
{
    // check if the send_message table has a free entry
    // if yes, then add the message to the table in the first free entry and return
    // if no, then wait for a free entry to be available

    // check if the send_message table has a free entry

    // if no, then wait for a free entry to be available


    pthread_mutex_lock(&send_table_mutex);
    while(mysocket.send_message->num_full == MAX_TABLE_SIZE){
        pthread_mutex_unlock(&send_table_mutex);
        printf("[my_send]: Waiting for a free entry in the send table ...\n");
        sleep(1);
        pthread_mutex_lock(&send_table_mutex);
    }
    pthread_mutex_unlock(&send_table_mutex);
    
    // printf("[my_send]: Found a free entry in the send table!\n");
    // if yes, then add the message to the table in the first free entry and return
    int free_entry = -1;
    pthread_mutex_lock(&send_table_mutex);
    for (int i = 0; i < MAX_TABLE_SIZE; i++){
        if (mysocket.send_message->messages[i].flag == 0){
            free_entry = i;
            break;
        }
    }
    
    if (free_entry != -1){

        if (len > MAX_MESSAGE_SIZE){
            printf("[my_send]: Message too large!\n");
            return -1;
        }

        mysocket.send_message->messages[free_entry].length = len;


        // memcpy(mysocket.send_message->messages[free_entry].data, buf, len);
        
        // don't use memcpy as it will stop copying when it encounters a '\0' character
        for (int i = 0; i < len; i++){
            mysocket.send_message->messages[free_entry].data[i] = ((char *)buf)[i];
            // printf("%c", mysocket.send_message->messages[free_entry].data[i]);
        }
        printf("\n");

        mysocket.send_message->messages[free_entry].flag = 1;
        mysocket.send_message->num_full++;
        pthread_mutex_unlock(&send_table_mutex);
        printf("[my_send]: Message added to the send table at index %d\n", free_entry);
        return len;
    }
    else{
        printf("[my_send]: No free entry found in the send table!\n");
        pthread_mutex_unlock(&send_table_mutex);
        return -1;
    }

}

void* recv_message(void* sockfd){

    int recv_sock = *((int*)sockfd);
    printf("[ Recv Thread ] Thread R has been created!\n");
    int bytes_recv = 0, total_bytes_recv = 0;
    size_t length = 0;

    int it = 0;
    pthread_mutex_lock(&connect_flag_mutex);
    while(connect_flag == -1){
        pthread_mutex_unlock(&connect_flag_mutex);
        if (it == 0){
            printf("[ Recv Thread ]: Waiting for connection to be established ...\n");
            it = 1;
        }
        // printf("Waiting for connection to be established ...\n");
        sleep(1);
        pthread_mutex_lock(&connect_flag_mutex);
    }
    pthread_mutex_unlock(&connect_flag_mutex);

    printf("[ Recv Thread ] Thread R woke up!\n");

    if (newfd != -1){
        recv_sock = newfd;
        // printf("[ Recv Thread ]: New socket created for receiving messages!\n");
    }

    while(1){
        
        length = 0;
        total_bytes_recv = 0; bytes_recv = 0;
        char *recv_buffer = (char *)malloc((MAX_MESSAGE_SIZE + 4)*sizeof(char));  // +1 for null character
        // receive the message in chunks, till the full message is received


        while(1){

            bytes_recv = recv(recv_sock, recv_buffer + total_bytes_recv, MAX_MESSAGE_SIZE + 4 - total_bytes_recv, 0);
            // printf("[ Recv Thread ]: Bytes received = %d\n", bytes_recv);
            if (bytes_recv < 0){
                perror("[ Recv Thread ] Unable to receive a message!\n");
                exit(EXIT_FAILURE);
            }

            if (bytes_recv == 0){
                printf("[ Recv Thread ]: Connection closed by the other side!\n");
                printf("[ Recv Thread ]: Exiting ...\n");

                // // close the socket
                // if (close(recv_sock) < 0){
                //     perror("[ Recv Thread ]: Unable to close the socket!\n");
                //     exit(EXIT_FAILURE);
                // }
                // printf("[ Recv Thread ]: Socket closed!\n");
                pthread_exit(NULL);
            }
            total_bytes_recv += bytes_recv;
            if (total_bytes_recv < 4)
                continue;
            else{
                // for (int i = 0; i < 4; i++){
                //     printf("%d ", recv_buffer[i]);
                // }
                length = 0;
                length = (recv_buffer[0] - '0')*1000 + (recv_buffer[1] - '0')*100 + (recv_buffer[2] - '0')*10 + (recv_buffer[3] - '0');
                // pthread_mutex_lock(&stdout_mutex);

               
                // printf("[ Recv Thread ]: Length of the message = %ld\n", length);
                // pthread_mutex_unlock(&stdout_mutex);
                if (total_bytes_recv == length + 4)
                    break;
            }
        }

        printf("[ Recv Thread ] Received a message of size %d bytes!\n", total_bytes_recv - 4);

        // add the received message to the receive table, if there is a free entry
        // else, wait till there is a free entry
        pthread_mutex_lock(&recv_table_mutex);
        while(mysocket.recv_message->num_full == MAX_TABLE_SIZE){
            pthread_mutex_unlock(&recv_table_mutex);
            printf("[ Recv Thread ] Receive Table is full! Waiting for a free entry ...\n");
            sleep(1);
            pthread_mutex_lock(&recv_table_mutex);
        }
        pthread_mutex_unlock(&recv_table_mutex);

        // printf("[ Recv Thread ] Receive Table has a free entry! Adding the received message to it ...\n");

        // now, there is a free entry in the receive table, so add the received message to it
        pthread_mutex_lock(&recv_table_mutex);
        for (int i = 0; i < MAX_TABLE_SIZE; i++){
            if (mysocket.recv_message->messages[i].flag == 0){
                mysocket.recv_message->messages[i].flag = 1;
                mysocket.recv_message->messages[i].length = length;
                // mysocket.recv_message->messages[i].data = (char *)malloc((length)*sizeof(char));
                // memset(mysocket.recv_message->messages[i].data, 0, length);
                for (int j = 0; j < length; j++){
                    mysocket.recv_message->messages[i].data[j] = recv_buffer[j+4];
                }
                mysocket.recv_message->num_full++;
                printf("[ Recv Thread ] Added the received message to the Receive Table at index %d!\n", i);
                break;
            }
        }
        pthread_mutex_unlock(&recv_table_mutex);
        free(recv_buffer);
    }

}

int my_recv(int sockfd, void *buf, size_t len, int flags)
{
    // checks if the recv table has a message to be read
    // if yes, then read the message and return
    // if no, then wait for a message to be available

    pthread_mutex_lock(&recv_table_mutex);
    while(mysocket.recv_message->num_full == 0){
        pthread_mutex_unlock(&recv_table_mutex);
        printf("[my_recv]: Waiting for a message to be received in the recv table ...\n");
        sleep(1);
        pthread_mutex_lock(&recv_table_mutex);
    }
    pthread_mutex_unlock(&recv_table_mutex);

    int message_to_read = -1;
    pthread_mutex_lock(&recv_table_mutex);
    for (int i = 0; i < MAX_TABLE_SIZE; i++){
        if (mysocket.recv_message->messages[i].flag == 1){
            // pthread_mutex_lock(&stdout_mutex);
            size_t length = mysocket.recv_message->messages[i].length;
            printf("[my_recv]: Found a message to be read at index %d\n", i);
            // printf("[my_recv]: Client Message Length: %ld\n", length);
            // printf("[my_recv]: Client Message: %s\n\n", mysocket.recv_message->messages[i].data);
            // pthread_mutex_unlock(&stdout_mutex);
            
            // copy the message to the buffer
            for (int j = 0; j < mysocket.recv_message->messages[i].length; j++){
                ((char *)buf)[j] = mysocket.recv_message->messages[i].data[j];
            }

            // clear the recv table entry
            mysocket.recv_message->messages[i].length = 0;
            mysocket.recv_message->messages[i].flag = 0;
            mysocket.recv_message->num_full--;
            memset(mysocket.recv_message->messages[i].data, 0, MAX_MESSAGE_SIZE);

            pthread_mutex_unlock(&recv_table_mutex);
            return length;
            
        }
    }


}


int my_socket(int domain, int type, int protocol)
{
    // register SIGINT handler
    signal(SIGINT, sigint_handler);
    memset(&mysocket, 0, sizeof(mysocket));
    if (type == SOCK_MyTCP){

        if ((mysocket.sockfd = socket(domain, SOCK_MyTCP, protocol)) < 0) {
            perror("Cannot create the socket!\n");
            return mysocket.sockfd;
        }   

        // initialize the locks for the send and receive buffers
        printf("Initializing locks ...\n");
        pthread_mutex_init(&send_table_mutex, NULL);
        pthread_mutex_init(&recv_table_mutex, NULL);
        pthread_mutex_init(&connect_flag_mutex, NULL);
        pthread_mutex_init(&stdout_mutex, NULL);

        // create threads
        printf("Creating threads ...\n");
        pthread_create(&mysocket.R, NULL, recv_message, &(mysocket.sockfd));
        pthread_create(&mysocket.S, NULL, send_message, &(mysocket.sockfd));

        // pthread_create(&mysocket.R, NULL, recv_message, &newfd);
        // pthread_create(&mysocket.S, NULL, send_message, &newfd);

        // int num_messages = MAX_MESSAGE_SIZE/MAX_SEND_SIZE;
        // mysocket.Send_Message = (char **)malloc(num_messages * sizeof(char *));
        // mysocket.Received_Message = (char **)malloc(num_messages * sizeof(char *));

        // for (int i = 0; i < num_messages; i++) {
        //     mysocket.Send_Message[i] = (char *)malloc(MAX_SEND_SIZE * sizeof(char));
        //     mysocket.Received_Message[i] = (char *)malloc(MAX_SEND_SIZE * sizeof(char));
        // }

        // initialize and allocate memory for send and receive buffers
        printf("Initializing buffers ...\n");
        mysocket.send_message = (data_table *)malloc(sizeof(data_table));
        mysocket.recv_message = (data_table *)malloc(sizeof(data_table));
        mysocket.send_message->num_entries = MAX_TABLE_SIZE;
        mysocket.recv_message->num_entries = MAX_TABLE_SIZE;
        mysocket.send_message->messages = (message *)malloc(MAX_TABLE_SIZE * sizeof(message));
        mysocket.recv_message->messages = (message *)malloc(MAX_TABLE_SIZE * sizeof(message));
        mysocket.send_message->num_full = 0;
        mysocket.recv_message->num_full = 0;

        for (int i = 0; i < MAX_TABLE_SIZE; i++){
            mysocket.send_message->messages[i].length = 0;
            mysocket.send_message->messages[i].data = (char *)malloc(MAX_MESSAGE_SIZE * sizeof(char));
            mysocket.send_message->messages[i].flag = 0;
            mysocket.recv_message->messages[i].length = 0;
            mysocket.recv_message->messages[i].data = (char *)malloc(MAX_MESSAGE_SIZE * sizeof(char));
            mysocket.recv_message->messages[i].flag = 0;
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
    int newsockfd;
    if ((newsockfd = accept(sockfd, addr, addrlen)) < 0) {
        perror("Unable to accept connection!\n");
    }

    
    // set the connect flag to 1
    pthread_mutex_lock(&connect_flag_mutex);
    newfd = newsockfd;
    connect_flag = 1;
    // mysocket.sockfd = newsockfd;
    pthread_mutex_unlock(&connect_flag_mutex);

  
 
    return newsockfd;
}

int my_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    int connect_status;
    if ((connect_status = connect(sockfd, addr, addrlen)) < 0) {
        perror("Unable to connect to remote host!\n");
    }

    // newfd = sockfd;

    pthread_mutex_lock(&connect_flag_mutex);
    connect_flag = 1;
    pthread_mutex_unlock(&connect_flag_mutex);

    return connect_status;
}



int my_close(int sockfd)
{
    int close_status;
    sleep(10);

    if ((close_status = close(sockfd)) < 0) {
        perror("Unable to close socket!\n");
    }

    /*
        clear contents of mysocket
        this also deletes the sockfd stored in it
        done because at the end, this fd will be the last one to refer to the actual socket
        so it should be closed
    */

    // free buffers
    printf("Freeing buffers ...\n");
    for (int i = 0; i < MAX_TABLE_SIZE; i++){
        // printf("Freeing send buffer %d ...\n", i);
        free(mysocket.send_message->messages[i].data);
        // printf("Freeing recv buffer %d ...\n", i);
        free(mysocket.recv_message->messages[i].data);
    }
    printf("Freeing tables ...\n");
    free(mysocket.send_message->messages);
    free(mysocket.recv_message->messages);
    free(mysocket.send_message);
    free(mysocket.recv_message);


    // join threads
    printf("Joining threads ...\n");
    // kill the R and S thread by sending a signal
    pthread_cancel(mysocket.R);
    pthread_cancel(mysocket.S);
    
    // join the threads
    pthread_join(mysocket.R, NULL);
    pthread_join(mysocket.S, NULL);


    // int num_messages = MAX_MESSAGE_SIZE/MAX_SEND_SIZE;
    // for (int i = 0; i < num_messages; i++) {
    //     free(mysocket.Send_Message[i]);
    //     free(mysocket.Received_Message[i]);
    // }
    // free(mysocket.Send_Message);
    // free(mysocket.Received_Message);



    // clear mysocket
    printf("Clearing mysocket ...\n");
    memset(&mysocket, 0, sizeof(mysocket));         
    return close_status;
}

