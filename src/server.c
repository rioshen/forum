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
#include <stddef.h>

#include "util.h"
#include "server.h"
#include "fdb.h"


int write_sock(int sock, char *buf, size_t len) {
    ssize_t byteswrote = 0;
    ssize_t ret = 0;

    while (byteswrote < len)
    {
        ret = send(sock, buf + byteswrote, len - byteswrote, 0);

        if (ret < 0)
        {
            return -1;
        }

        if (ret == 0)
        {
            break;
        }

        byteswrote += ret;
    }

    return byteswrote;
}

int read_sock(int sock, char *buf, size_t len) {
    ssize_t ret = 0;
    ssize_t bytesread = 0;

    printf("Runs in read_sock, buffer is %s", buf);

    while (bytesread < len)
    {
        ret = recv(sock, buf + bytesread, len - bytesread, 0);

        if (ret == 0)
        {
            break;
        }

        if (ret < 0)
        {
            return -1;
        }

        bytesread += ret;
    }

    return bytesread;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc) {
    int read_size = 0;
    int sock = *(int*)socket_desc;
    char *message = NULL;
    char client_message[2000];

    while ( (read_size = recv(sock , client_message , 2000 , 0)) > 0 ) {
        struct Action *action = (struct Action *)client_message;

        printf("Receive message %s\n", action->cmd);
        printf("Filed 1 is %s\n", action->filed1);
        printf("Filed 2 is %s\n", action->filed2);

        if (strncmp(action->cmd, CMD_LOGIN, strlen(CMD_LOGIN)) == 0) {
            if (authentication(action->filed1, action->filed2) == FORUM_OK) {
                send(sock, "AUTH SUCCESS", strlen("AUTH_SUCCESS"), 0);
            } else {
                send(sock, AUTH_FAILED, strlen(AUTH_FAILED), 0);
            }
        } else if ((strncmp(action->cmd, CMD_ADDPOST, strlen(CMD_ADDPOST))) == 0) {
            if (add_post(action->filed1, action->filed2) == FORUM_OK) {
                send(sock, POST_SUCCESS, strlen(POST_SUCCESS), 0);
            } else {
                send(sock, POST_FAILED, strlen(POST_FAILED), 0);
            }
        } else if ((strncmp(action->cmd, CMD_DISPLAY, strlen(CMD_DISPLAY))) == 0) {
            char *buffer = (void *)malloc(1024 + 1);
            get_post(buffer);
            printf("Result: %s", buffer);
            send(sock, buffer, strlen(buffer), 0);
            free(buffer);
       } else if ((strncmp(action->cmd, CMD_SHOW_POST, strlen(CMD_SHOW_POST))) == 0) {
            char *buffer = (void *)malloc(1024 + 1);
            show_post(action->filed1, buffer);
            send(sock, buffer, strlen(buffer), 0);
            free(buffer);
       }


//        if (strncmp(client_message, "admin", strlen("admin")) == 0) {
//            if (authentication("admin", "admin") == FORUM_OK) {
//
//            } else {
//                write(sock, "AUTH FAILED", strlen("AUTH_FAILED"));
//            }
//        }

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
    server.sin_port = htons(PORTNUM);

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
