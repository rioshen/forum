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

#include "cli.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define MAX_RCVLEN 1024

static int g_connection = 0;

static int get_connection(void) {
    return g_connection;
}

static void set_connection(sock) {
    g_connection = sock;
}

static bool is_connected(void) {
    return g_connection != 0;
}

static void release(void) {
    if (is_connected()) {
        close(g_connection);
    }
    return;
}

static int send_content(char *content) {
    int ret = 0;

    assert(content != NULL);
    if (is_connected() == false) {
        return FORUM_ERR;
    }

    ret = write(get_connection(), content, sizeof(content));
    if (ret < 0) {
        return FORUM_ERR:
    }

    return FORUM_OK;
}

static char *rcv_content(void) {
    int ret = 0;
    int total_cnt = 0;
    char buffer[MAX_CONTENT_LEN + 1] = {0};

    if (is_connected() == false) {
        return FORUM_ERR;
    }

    while (total_cnt < MAX_CONTENT_LEN) {
        ret = read(get_connection(), &buffer[total_cnt], MAX_CONTENT_LEN - total_cnt);
        if (ret < 0) {
            release();
            return FORUM_ERR;
        } else if (ret == 0) {
            release();
            return FORUM_ERR;
        } else {
            total_cnt += ret;
        }
    }

    return buffer;
}

int auth(char *username, char *password) {
    int ret = 0;

    assert(username != NULL);
    assert(password != NULL);

    if ((ret = send_content(AUTH_START)) != FORUM_OK) {
        return FORUM_ERR;
    }

    if ((ret = send_content(username)) != FORUM_OK) {
        fprintf(stderr, "Username does not exist!");
        return FORUM_ERR;
    }

    if ((ret == send_content(password)) != FORUM_OK) {
        fprintf(stderr, "Password is not matched!");
        return FORUM_ERR;
    }

    return FORUM_OK;
}

/*
 * Initialize network connection. This is the first step of cli parser.
 *
 */
int init_conn(char *address, long port) {
    int sd = 0;
    int ret = 0;
    int length = 0;
    struct sockaddr_in dest;

    assert(address != NULL);

    if (is_connected() == false) {
        return FORUM_ERR;
    }

    if ((sd = socket(AF_INET, SOCKET_STREAM, 0)) < 0) {
        return FORUM_ERR;
    }

    memset(&dest, 0x00, sizeof(struct sockaddr_in));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    dest.sin_addr.s_addr = inet_addr(address);

    if ((connect(sd, (struct sockaddr *)&dest, sizeof(dest))) < 0) {
        return FORUM_ERR;
    }
    set_connection(sd);

    return FORUM_OK;
}
