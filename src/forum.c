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
#include <syslog.h>
#include <stddef.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "forum.h"
#include "server.h"
#include "cli.h"

#define COMMAND_LEN     1024
#define USER_NAME_LEN   1024
#define PASSWORD_LEN    1024
#define EXIT_CMD_LEN    4
#define TITTLE_LEN      128
#define POST_MAX_LEN    1024
#define SEPERATOR       "SEPERATOR"
#define RESPONSE_LEN    128

char g_user_name[USER_NAME_LEN + 1] = {0};
char g_password[PASSWORD_LEN + 1] = {0};

void usage(void) {
    fprintf(stderr, "Usage: ./forum_server [options]\n");
    fprintf(stderr, "       ./forum-server -h or --help\n");
    fprintf(stderr, "       ./forum_server --port 777\n");
    exit(1);
}

int read_sock(int sock, char *buf, size_t len) {
    ssize_t ret = 0;
    ssize_t bytesread = 0;

    printf("Runs in read_sock, buffer is %s\n", buf);

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

int main(int argc, char**argv) {
    int sock;
    struct sockaddr_in server;
    char command[COMMAND_LEN + 1]  = {0};
    char server_reply[2000] = {0};
    struct Action action;
    char message[sizeof(action)];

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
    server.sin_port = htons(PORTNUM);
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
            if (fgets(g_user_name, USER_NAME_LEN, stdin) == NULL) {
                fprintf(stderr, "Failed to get user name.");
                continue;
            }
            g_user_name[strlen(g_user_name) - 1] = '\0';

            printf("password:> ");
            if (fgets(g_password, PASSWORD_LEN, stdin) == NULL) {
                fprintf(stderr, "Failed to get password");
                continue;
            }
            g_password[strlen(g_password) - 1] = '\0';

            memset(&action, 0, sizeof(action));
            strncpy(action.cmd, CMD_LOGIN, strlen(CMD_LOGIN));
            strncpy(action.filed1, g_user_name, strlen(g_user_name));
            strncpy(action.filed2, g_password, strlen(g_password));
            if ((send(sock, (char *)&action, sizeof(action), 0)) < 0) {
                fprintf(stderr, "Failed to send action");
                exit(-1);
            }

            if ((recv(sock, server_reply, strlen(AUTH_SUCCESS), 0)) < 0) {
                fprintf(stderr, "Failed to get response.");
                exit(-1);
            }

            if ((strncmp(server_reply, AUTH_SUCCESS, strlen(AUTH_SUCCESS))) != 0) {
                printf("%s\n", server_reply);
                continue;
            } else {
                printf("Auth sucess!\n");
                continue;
            }
        } else if ((strncmp(command, "add", strlen("add"))) == 0) {
            char title[TITTLE_LEN + 1] = {0};
            char content[POST_MAX_LEN + 1] = {0};

            printf("title:> ");
            if (fgets(title, TITTLE_LEN, stdin) == NULL) {
                fprintf(stderr, "Failed to get post title.");
                continue;
            }
            title[strnlen(title, TITTLE_LEN) - 1] = '\0';

            printf("content:> ");
            if (fgets(content, POST_MAX_LEN, stdin) == NULL) {
                fprintf(stderr, "Failed to get post content.");
                continue;
            }
            content[strnlen(content, POST_MAX_LEN) - 1] = '\0';

            memset(&action, 0, sizeof(action));
            strncpy(action.cmd, CMD_ADDPOST, strlen(CMD_ADDPOST));
            strncpy(action.filed1, title, strlen(title));
            strncpy(action.filed2, content, strlen(content));

            if ((send(sock, (char *)&action, sizeof(action), 0)) < 0) {
                fprintf(stderr, "Failed to send action");
                exit(-1);
            }

            if ((recv(sock, server_reply, strlen(POST_SUCCESS), 0)) < 0) {
                fprintf(stderr, "Failed to get response.");
                exit(-1);
            }

            if ((strncmp(server_reply, POST_SUCCESS, strlen(POST_SUCCESS))) != 0) {
                fprintf(stderr, "Failed to post");
                continue;
            } else {
                printf("Post success");
                continue;
            }
        } else if ((strncmp(command, "display", strlen("display"))) == 0) {
            memset(&action, 0, sizeof(action));
            strncpy(action.cmd, CMD_DISPLAY, strlen(CMD_DISPLAY));
            if ((send(sock, (char *)&action, sizeof(action), 0)) < 0) {
                fprintf(stderr, "Failed to send action");
                exit(-1);
            }

            memset(server_reply, 0, sizeof(server_reply));
            if ((recv(sock, server_reply, sizeof(server_reply), 0)) < 0) {
                fprintf(stderr, "Failed to get response.");
                exit(-1);
            }
            printf("%s", server_reply);
            printf("Article number:> ");

            if ((fgets(command, COMMAND_LEN, stdin)) == NULL) {
                fprintf(stderr, "Empty command.");
                continue;
            }
            command[strnlen(command, COMMAND_LEN) - 1] = '\0';
            memset(&action, 0, sizeof(action));
            strncpy(action.cmd, CMD_SHOW_POST, strlen(CMD_SHOW_POST));
            strncpy(action.filed1, command, strlen(command));

            if ((send(sock, (char *)&action, sizeof(action), 0)) < 0) {
                fprintf(stderr, "Failed to send action");
                exit(-1);
            }

            memset(server_reply, 0, sizeof(server_reply));
            if ((recv(sock, server_reply, sizeof(server_reply), 0)) < 0) {
                fprintf(stderr, "Failed to get response.");
                exit(-1);
            }
            printf("%s\n", server_reply);
        }

    }

    return 0;
}
