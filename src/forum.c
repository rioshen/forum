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
#include "forum.h"
#include "server.h"
#include "cli.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#define COMMAND_LEN     1024
#define USER_NAME_LEN   1024
#define PASSWORD_LEN    1024
#define POST_MAX_LEN    1024
#define EXIT_CMD_LEN    4

int is_auth = 0;
int g_sockfd = 0;
char g_user_name[USER_NAME_LEN + 1] = {0};
char g_password[PASSWORD_LEN + 1] = {0};

void repl() {
    int ret = 0;
    char command[COMMAND_LEN] = {0};

    while (1) {
        printf("forum:> ");

        if (fgets(command, COMMAND_LEN, stdin) == NULL) {
            fprintf(stderr, "Invalid command.\n");
            exit(1);
        }

        if (strncmp(command, "exit", strlen("exit")) == 0) {
            exit(1);
        }

        if (strncmp(command, "login", strlen("login")) == 0) {
            exit(1);
        }

        if (strncmp(command, "post", strlen("post")) == 0) {
            exit(1);
        }

        if (strncmp(command, "show", strlen("show")) == 0) {
            exit(1);
        }

    }

    return;
}

void usage(void) {
    fprintf(stderr, "Usage: ./forum_server [options]\n");
    fprintf(stderr, "       ./forum-server -h or --help\n");
    fprintf(stderr, "       ./forum_server --port 777\n");
    exit(1);
}

int main(int argc, char**argv) {
    int sock;
    struct sockaddr_in server;
    char command[COMMAND_LEN + 1]  = {0};
    char server_reply[2000] = {0};

    if (argc >= 2) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            usage();
        }
    }

    if ((sock = socket(AF_INET , SOCK_STREAM , 0)) < 0) {
        fprintf(stderr, "Failed to creak socket.");
        exit(-1);
    }

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(5001);
    if ((connect(sock , (struct sockaddr *)&server , sizeof(server))) < 0) {
        fprintf(stderr, "Failed to connect to server.");
        exit(-1);
    }

    openlog("Forum Client", 0, LOG_LOCAL0);
    syslog(LOG_INFO, "%s", "Initialization success.");

    while (1) {
        printf("forum:> ");
        if (fgets(command, COMMAND_LEN, stdin) == NULL) {
            fprintf(stderr, "Invalid command.\n");
            exit(1);
        }

        if (strncmp(command, "login", strlen("login")) == 0) {
            printf("username:> ");

            (void)fgets(g_user_name, USER_NAME_LEN, stdin);
            if ((send(sock, g_user_name, strlen(g_user_name), 0)) < 0) {
                fprintf(stderr, "Failed to send user name.");
                exit(-1);
            }

            char message[strlen(AUTH_SUCCESS)] = {0};
            if ((recv(sock, message, strlen(AUTH_SUCCESS), 0)) < 0) {
                fprintf(stderr, "Failed to get response.");
                exit(-1);
            }
            printf("%s\n", message);
            if (strncmp(message, AUTH_SUCCESS, strlen(AUTH_SUCCESS)) != 0) {
                fprintf(stderr, "Use name does not exit.");
                break;
            }

            printf("password:> ");
            (void)fgets(g_password, PASSWORD_LEN, stdin);
            if ((send(sock, g_password, strlen(g_password), 0)) < 0) {
                fprintf(stderr, "Failed to send user name.");
                exit(-1);
            }
            memset(message, 0, strlen(message));
            if ((recv(sock, message, strlen(AUTH_SUCCESS), 0)) < 0) {
                fprintf(stderr, "Failed to get response.");
                exit(-1);
            }
            if (strncmp(message, AUTH_SUCCESS, strlen(AUTH_SUCCESS)) != 0) {
                fprintf(stderr, "Use name does not exit.");
                break;
            }
        }

    }

    return 0;
}
