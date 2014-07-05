#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>

#include "server.h"
#include "fdb.h"

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc) {
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , client_message[2000];

    //Receive a message from client
    while ( (read_size = recv(sock , client_message , 2000 , 0)) > 0 ) {
        printf("Receive message %s\n", client_message);

        if (strncmp(client_message, "admin", strlen("admin")) == 0) {

            write(sock, "AUTH SUCCESS", strlen("AUTH_SUCCESS"));
        }
    }

    if (read_size == 0) {
        puts("Client disconnected");
        fflush(stdout);
    } else if (read_size == -1) {
        perror("recv failed");
    }

    free(socket_desc);

    return 0;
}

int main(int argc , char *argv[]) {
    int ret = 0;
    int socket_desc , new_socket , c , *new_sock;
    struct sockaddr_in server , client;
    char *message;

    if ((ret = init_database()) != FORUM_OK) {
        fprintf(stderr, "Initialize database failed.");
        return -1;
    }

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)    {
        printf("Could not create socket");
    }

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 5001 );

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0) {
        puts("bind failed");
        return 1;
    }
    puts("bind done");

    //Listen
    listen(socket_desc , 3);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) ) {
        puts("Connection accepted");

        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = new_socket;

        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0) {
            perror("could not create thread");
            return 1;
        }
        puts("Handler assigned");
    }

    if (new_socket < 0) {
        perror("accept failed");
        return 1;
    }

    return 0;
}
