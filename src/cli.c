#include "cli.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COMMAND_LEN     1024
#define EXIT_CMD_LEN    4

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
            printf("forum:> password: ");
            if (fgets(command, COMMAND_LEN, stdin) != NULL) {
                ret =
            }
        }
    }

    return;
}
