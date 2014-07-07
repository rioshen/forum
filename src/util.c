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

int get_file_content(char *file_name, char *buffer) {
    int ret = 0;
    FILE *file = NULL;

    if (file_name == NULL || buffer == NULL) return FORUM_ERR;

    if ((file = fopen(file_name, "rb")) == NULL) return FORUM_ERR;

    if ((ret = fread(buffer, 1, MAX_BUFF_LEN, file)) < 1) return FORUM_ERR;

    fclose(file);

    return FORUM_OK;
}

int set_file_content(int size, char *name, char *path, char *content) {
    int offset = 0;
    FILE *file = NULL;
    char file_name[FILE_NAME_LEN + 1] = {0};

    if (name == NULL || path == NULL || content == NULL) {
        return FORUM_ERR;
    }

    snprintf(file_name, "%s%s", FILE_PREFIX, name);
    printf("recv file %s\n", file_name);
    file = fopen(file_name, "wb");
    if (file == NULL) {
        return FORUM_ERR;
    }

    do {
        int written = fwrite(content, 1, size - offset, file);
        if (written < 1) {
            fclose(file);
            return FORUM_ERR;
        }
        offset += written;
    } while (offset < size);

    fclose(file);
    strncpy(path, file_name, FILE_NAME_LEN);
    return FORUM_OK;
}
