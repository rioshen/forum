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

typedef struct {
    char *title;
    char *content;
    char *author;
}post;

struct {
    char *username;
    char *password;
}account;

#define COMMAND_LEN     1024
#define USER_NAME_LEN   1024
#define PASSWORD_LEN    1024
#define POST_MAX_LEN    1024
#define EXIT_CMD_LEN    4

bool is_auth = false;
char g_user_name[USER_NAME_LEN + 1] = {0};
char g_password[PASSWROD_LEN + 1] = {0};

static bool is_auth(void) {
    return is_auth;
}

static int show_post(void) {
    char *post[POST_MAX_LEN + 1] = {0};

    if (!is_auth()) {
        fprintf(stderr, "Please login first. To do this, type login.\n");
        return FORUM_ERR;
    }


    if ((post = retrive_post(g_user_name)) == NULL) {
        return FORUM_ERR:
    }

    printf("Post by %s:> \n", g_user_name);
    printf("%s\n", post);
    printf("forum:>");

    return FORUM_OK;

}

static int create_post(void) {
    return FORUM_OK;
}

static int auth_cli(void) {

    struct account *user = NULL;

    printf("username:> ");
    if (fgets(g_user_name, USER_NAME_LEN, stdin) == NULL) {
        return FORUM_ERR;
    }

    printf("password:> ");
    if (fgets(g_password, PASSWORD_LEN, stdin) == NULL) {
        return FORUM_ERR;
    }

    if (auth(g_user_name, g_password) != FORUM_OK) {
        return FORUM_ERR;
    }
    is_auth = true;

    return FORUM_OK;
}

void repl(void) {
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
            if (aut_cli() != FORUM_OK) {
                printf("forum:> Authentication failed.\n");
            }
        }

        if (strncmp(command, "post", strlen("post")) != NULL) {
            if (create_post() != FORUM_OK) {
                printf("forum:> Failed to create a new post.\n");
            }
        }

        if (strncmp(command, "show", strlen("show") != NULL)) {
            if (show_post() != FORUM_OK) {
                printf("forum:> Failed to find a specific post.\n");
            }
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
    char *ptr = NULL;
    long port = 0;

    if (argc >= 2) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            usage();
        }
        if (strcmp(argv[1], "--port") == 0 && argc == 3) {
            if (strlen(argv[2]) > 5) {
                fprintf(stderr, "Invalid port number.\n");
                exit(1);
            } else {
                port = strtol(argv[2], &ptr, 10);
                printf("Port number is %d\n", port);
            }
        }
    }

    repl();

    return 0;
}
