#include "forum.h"
#include "server.h"
#include "cli.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void usage(void) {
    fprintf(stderr, "Usage: ./forum_server [options]\n");
    fprintf(stderr, "       ./forum-server -h or --help\n");
    fprintf(stderr, "       ./forum_server --port 777\n");
    exit(1);
}

void run_server(int port) {
    printf("Runs here\n");
    repl();
    return;
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

    run_server((int)port);

    return 0;
}
