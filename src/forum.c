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
#include <ctype.h>
#include <stddef.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "util.h"
#include "server.h"

#define COMMAND_LEN     1024
#define USER_NAME_LEN   1024
#define PASSWORD_LEN    1024
#define EXIT_CMD_LEN    4
#define TITTLE_LEN      128
#define POST_MAX_LEN    1024
#define RESPONSE_LEN    2048
#define PROMPT_LEN      128

static int g_login = 0;

void usage(void) {
    fprintf(stderr, "Usage: ./forum_server [options]\n");
    fprintf(stderr, "       ./forum-server -h or --help\n");
    fprintf(stderr, "       ./forum_server --port 777\n");
    exit(1);
}

/**
 * Check whether a file is legal, which means the file should be only text file
 * and also it is not a symlink file.
 */
static int file_validation(const char *file_name) {

    // Check whether its extension is .txt
    const char* dot = strchr(file_name, '.');
    if (!dot || dot == file_name) return FORUM_ERR;
    if (strncmp((dot + 1), "txt", strlen("txt")) != 0) {
        fprintf(stderr, "%s\n", "Unsupported file type.");
        return FORUM_ERR;
    }

    // check whether it is a symbolic link file
    if ((check_symlink(file_name)) != FORUM_OK) {
        fprintf(stderr, "%s\n", "File is a symbolic link.");
        return FORUM_ERR;
    }

    return FORUM_OK;
}

/**
 * To avoid SQL injection, input data could only contain digit or alphabetic.
 */
static int input_validation(const char *data) {
    int i = 0;

    if (data == NULL) {
        return FORUM_ERR;
    }

    int length = strnlen(data, COMMAND_LEN);
    for (i = 0; i < length; i++) {
        if (isdigit((int)data[i]) || isalpha((int)data[i]) || data[i] == ' ') {
            continue;
        } else {
            return FORUM_ERR;
        }
    }

    return FORUM_OK;
}

int message_handler(int sock, char *cmd, char *field1, char *field2) {
    struct Action action;
    char response[RESPONSE_LEN + 1] = {0};

    if (cmd == NULL || field1 == NULL || field2 == NULL) {
        return FORUM_ERR;
    }

    memset(&action, 0, sizeof(action));
    strncpy(action.cmd, cmd, strnlen(cmd, CMD_MAX_LEN));
    strncpy(action.field1, field1, strnlen(field1, FIELD_ONE_LEN));
    strncpy(action.field2, field2, strnlen(field2, FIELD_TWO_LEN));

    if ((send(sock, (char *)&action, sizeof(action), 0)) < 0) {
        fprintf(stderr, "Failed to send action");
        return FORUM_ERR;
    }

    if ((recv(sock, response, RESPONSE_LEN, 0)) < 0) {
        fprintf(stderr, "Failed to get response.");
        return FORUM_ERR;
    }

    if ((strncmp(response, OPT_SUCCESS, strlen(OPT_SUCCESS))) != 0) {
        return FORUM_ERR;
    }

    return FORUM_OK;
}

int main(int argc, char**argv) {
    int sock;
    struct sockaddr_in server;
    char command[COMMAND_LEN + 1]  = {0};
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

    while (1) {
        printf("forum:> ");
        if (fgets(command, COMMAND_LEN, stdin) == NULL) {
            fprintf(stderr, "Invalid command.\n");
            continue;
        }

        if ((strncmp(command, "exit", strlen("exit"))) == 0) {
            exit(-1);
        }

        if ((strncmp(command, "signup", strlen("signup"))) == 0) {
            char user_name[USER_NAME_LEN + 1] = {0};
            char password[PASSWORD_LEN + 1] = {0};

            printf("user name:> ");
            if (fgets(user_name, USER_NAME_LEN, stdin) == NULL) {
                fprintf(stderr, "Failed to get user name.\n");
                continue;
            }
            user_name[strlen(user_name) - 1] = '\0';
            if (input_validation(user_name) != FORUM_OK) {
                fprintf(stderr, "Input data could only contain digit or alpha\n");
                continue;
            }

            printf("password:> ");
            if (fgets(password, PASSWORD_LEN, stdin) == NULL) {
                fprintf(stderr, "Failed to get password");
                continue;
            }
            password[strlen(password) - 1] = '\0';
            if (input_validation(password) != FORUM_OK) {
                fprintf(stderr, "Input data could only contain digit or alpha\n");
                continue;
            }

            if ((message_handler(sock, CMD_SIGNUP, user_name, password)) != FORUM_OK) {
                fprintf(stderr, "Failed to sign up a new user.\n");
            } else {
                printf("Register success!\n");
            }
            continue;
        } else if (strncmp(command, "login", strlen("login")) == 0) {
            char user_name[USER_NAME_LEN + 1] = {0};
            char password[PASSWORD_LEN + 1] = {0};

            printf("user name:> ");
            if (fgets(user_name, USER_NAME_LEN, stdin) == NULL) {
                fprintf(stderr, "Failed to get user name.\n");
                continue;
            }
            user_name[strlen(user_name) - 1] = '\0';
            if (input_validation(user_name) != FORUM_OK) {
                fprintf(stderr, "Input data could only contain digit or alpha\n");
                continue;
            }

            printf("password:> ");
            if (fgets(password, PASSWORD_LEN, stdin) == NULL) {
                fprintf(stderr, "Failed to get password");
                continue;
            }
            password[strlen(password) - 1] = '\0';
            if (input_validation(password) != FORUM_OK) {
                fprintf(stderr, "Input data could only contain digit or alpha\n");
                continue;
            }

            if ((message_handler(sock, CMD_LOGIN, user_name, password)) != FORUM_OK) {
                fprintf(stderr, "User name or password is wrong.\n");
                g_login = 0;
            } else {
                printf("Login success!\n");
                g_login = 1;
            }
            continue;

        } else if ((strncmp(command, "post", strlen("post"))) == 0) {
            char title[TITTLE_LEN + 1] = {0};
            char content[POST_MAX_LEN + 1] = {0};

            if (g_login == 0) {
                fprintf(stderr, "Please login to post new article.\n");
                continue;
            }

            printf("title:> ");
            if (fgets(title, TITTLE_LEN, stdin) == NULL) {
                fprintf(stderr, "Failed to get post title.");
                continue;
            }
            title[strnlen(title, TITTLE_LEN) - 1] = '\0';
            if (input_validation(title) != FORUM_OK) {
                fprintf(stderr, "Input data could only contain digit or alpha\n");
                continue;
            }

            printf("content:> ");
            if (fgets(content, POST_MAX_LEN, stdin) == NULL) {
                fprintf(stderr, "Failed to get post content.");
                continue;
            }
            content[strnlen(content, POST_MAX_LEN) - 1] = '\0';
            if (input_validation(content) != FORUM_OK) {
                fprintf(stderr, "Input data could only contain digit or alpha\n");
                continue;
            }

            if ((message_handler(sock, CMD_ADDPOST, title, content)) != FORUM_OK) {
                fprintf(stderr, "Failed to add post entry.\n");
            } else {
                printf("Create post success!\n");
            }
            continue;

        } else if ((strncmp(command, "display", strlen("display"))) == 0) {
            char server_reply[CLIENT_MSG_LEN] = {0};
            if (g_login == 0) {
                fprintf(stderr, "Please login to post new article.\n");
                continue;
            }

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
            if (input_validation(command) != FORUM_OK) {
                fprintf(stderr, "Input data could only contain digit or alpha\n");
                continue;
            }

            memset(&action, 0, sizeof(action));
            strncpy(action.cmd, CMD_SHOW_POST, strlen(CMD_SHOW_POST));
            strncpy(action.field1, command, strnlen(command, COMMAND_LEN));

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
        } else if ((strncmp(command, "upload", strlen("upload"))) == 0) {
            char content[FIELD_TWO_LEN + 1] = {0};
            char file_name[FILE_NAME_LEN + 1] = {0};
            if (g_login == 0) {
                fprintf(stderr, "Please login to post new article.\n");
                continue;
            }

            printf("file name:> ");
            if ((fgets(file_name, FILE_NAME_LEN, stdin)) == NULL) {
                fprintf(stderr, "Failed to get post content.");
                continue;
            }
            file_name[strnlen(file_name, FILE_NAME_LEN) - 1] = '\0';
            if ((file_validation(file_name)) != FORUM_OK) {
                continue;
            }

            if ((get_file_content(file_name, content)) != FORUM_OK) {
                fprintf(stderr, "%s\n", "Failed to get file content");
                continue;
            }
            if ((message_handler(sock, CMD_UPLOAD, file_name, content)) != FORUM_OK) {
                fprintf(stderr, "%s\n", "Failed to send file content");
            } else {
                fprintf(stdout, "%s\n", "Upload file success!");
            }
            continue;
        } else if ((strncmp(command, "files", strlen("files"))) == 0) {
            char server_reply[CLIENT_MSG_LEN] = {0};
            if (g_login == 0) {
                fprintf(stderr, "Please login to post new article.\n");
                continue;
            }

            memset(&action, 0, sizeof(action));
            strncpy(action.cmd, CMD_FILE, strlen(CMD_FILE));
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

            printf("file number:> ");
            if ((fgets(command, COMMAND_LEN, stdin)) == NULL) {
                fprintf(stderr, "Empty command.");
                continue;
            }
            command[strnlen(command, COMMAND_LEN) - 1] = '\0';
            if (input_validation(command) != FORUM_OK) {
                fprintf(stderr, "Input data could only contain digit or alpha\n");
                continue;
            }

            memset(&action, 0, sizeof(action));
            strncpy(action.cmd, CMD_DOWNLOAD, strlen(CMD_DOWNLOAD));
            strncpy(action.field1, command, strnlen(command, COMMAND_LEN));

            if ((send(sock, (char *)&action, sizeof(action), 0)) < 0) {
                fprintf(stderr, "Failed to send action");
                exit(-1);
            }

            memset(server_reply, 0, sizeof(server_reply));
            int size = 0;
            char *path = (void *)malloc(FILE_NAME_LEN + 1);
            if ((size = recv(sock, server_reply, sizeof(server_reply), 0)) < 0) {
                fprintf(stderr, "Failed to get response.");
                exit(-1);
            }
            if ((set_file_content(size, "./download/", "download.txt", path, server_reply)) != FORUM_OK) {
                fprintf(stderr, "%s\n","Failed to download file.");
            } else {
                fprintf(stdout, "Download file %s success!\n", path);
            }
            free(path);
            continue;
        }
    }

    return 0;
}
