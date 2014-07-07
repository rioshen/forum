/*
 * Copyright © 2014
 * Rio Shen <rioxshen@gmail.com>
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>
 * or write to the Free Software Foundation, Inc., 51 Franklin St
 * Fifth Floor, Boston, MA 02110-1301 USA
 */

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

/**
 * This will handle connection for each client
 */
void *connection_handler(void *socket_desc) {
    int read_size = 0;
    int sock = *(int*)socket_desc;
    char client_message[CLIENT_MSG_LEN + 1];

    while ( (read_size = recv(sock , client_message , CLIENT_MSG_LEN , 0)) > 0 ) {
        struct Action *action = (struct Action *)client_message;

        if ((strncmp(action->cmd, CMD_SIGNUP, strlen(CMD_SIGNUP))) == 0) {
            if (action->field1 == NULL || action->field2 == NULL) {
                send(sock, OPT_FAILED, strlen(OPT_FAILED), 0);
                continue;
            }
            if (add_account(action->field1, action->field2) != FORUM_OK) {
                send(sock, OPT_FAILED, strlen(OPT_FAILED), 0);
            } else {
                send(sock, OPT_SUCCESS, strlen(OPT_SUCCESS), 0);
            }
            continue;

        } else if (strncmp(action->cmd, CMD_LOGIN, strlen(CMD_LOGIN)) == 0) {
            if (action->field1 == NULL || action->field2 == NULL) {
                send(sock, OPT_FAILED, strlen(OPT_FAILED), 0);
                continue;
            }
            if (authentication(action->field1, action->field2) == FORUM_OK) {
                send(sock, OPT_SUCCESS, strlen(OPT_SUCCESS), 0);
            } else {
                send(sock, OPT_FAILED, strlen(OPT_FAILED), 0);
            }
        } else if ((strncmp(action->cmd, CMD_ADDPOST, strlen(CMD_ADDPOST))) == 0) {
            if (action->field1 == NULL || action->field2 == NULL) {
                send(sock, OPT_FAILED, strlen(OPT_FAILED), 0);
                continue;
            }
            if (add_post(action->field1, action->field2) == FORUM_OK) {
                send(sock, OPT_SUCCESS, strlen(OPT_SUCCESS), 0);
            } else {
                send(sock, OPT_FAILED, strlen(OPT_FAILED), 0);
            }
        } else if ((strncmp(action->cmd, CMD_DISPLAY, strlen(CMD_DISPLAY))) == 0) {
            char *buffer = (void *)malloc(MAX_BUFF_LEN + 1);
            get_post(buffer);
            send(sock, buffer, MAX_BUFF_LEN, 0);
            free(buffer);
       } else if ((strncmp(action->cmd, CMD_SHOW_POST, strlen(CMD_SHOW_POST))) == 0) {
            char *buffer = (void *)malloc(MAX_BUFF_LEN + 1);
            show_post(action->field1, buffer);
            send(sock, buffer, MAX_BUFF_LEN, 0);
            free(buffer);
       } else if ((strncmp(action->cmd, CMD_UPLOAD, strlen(CMD_UPLOAD))) == 0) {
            char *path = (void *)malloc(FILE_NAME_LEN + 1);
            if ((set_file_content(read_size, FILE_PREFIX, action->field1, path, action->field2)) != FORUM_OK) {
                free(path);
                send(sock, OPT_FAILED, strlen(OPT_FAILED), 0);
                continue;
            }
            if ((add_file(action->field1, path)) != FORUM_OK) {
                send(sock, OPT_FAILED, strlen(OPT_FAILED), 0);
            } else {
                send(sock, OPT_SUCCESS, strlen(OPT_SUCCESS), 0);
            }
            free(path);
       } else if ((strncmp(action->cmd, CMD_FILE, strlen(CMD_FILE))) == 0) {
            char *buffer = (void *)malloc(MAX_BUFF_LEN + 1);
            get_file_list(buffer);
            send(sock, buffer, MAX_BUFF_LEN, 0);
            free(buffer);
       } else if ((strncmp(action->cmd, CMD_DOWNLOAD, strlen(CMD_DOWNLOAD))) == 0) {
            char *file_name = (void *)malloc(FILE_NAME_LEN + 1);
            char *buffer = (void *)malloc(FIELD_TWO_LEN + 1);
            get_file(action->field1, file_name);
            printf("file name is %s\n", file_name);
            if ((get_file_content(file_name, buffer)) != FORUM_OK) {
                send(sock, OPT_FAILED, strlen(OPT_FAILED), 0);
            } else {
                send(sock, buffer, FIELD_TWO_LEN, 0);
            }
            free(file_name);
            free(buffer);
       }
    }

    if (read_size == 0) {
        printf("Client disconnected\n");
        fflush(stdout);
    } else if (read_size == -1) {
        perror("Receive failed.\n");
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
        fprintf(stderr, "Initialize database failed.\n");
        return -1;
    }

    //Create socket
    if ((socket_desc = socket(AF_INET , SOCK_STREAM , 0)) == -1) {
        fprintf(stderr, "Initialize socket failed.\n");
        return -1;
    }

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORTNUM);

    //Bind
    if (bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0) {
        fprintf(stderr, "bind failed.\n");
        return -1;
    }

    //Listen
    listen(socket_desc , 3);

    //Accept and incoming connection
    c = sizeof(struct sockaddr_in);
    while ((new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c))) {
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = new_socket;

        if (pthread_create( &sniffer_thread , NULL,  connection_handler, (void*) new_sock) < 0) {
            perror("could not create thread");
            return 1;
        }
    }

    if (new_socket < 0) {
        perror("accept failed");
        return 1;
    }

    return 0;
}
