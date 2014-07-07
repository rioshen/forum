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
#include <sys/types.h>
#include <stddef.h>

#include "util.h"

/* use global variable to record uid and cover it back after file operations.*/
static uid_t euid, ruid;

/* Restore the effective UID to its original value. */
void do_setuid(void) {
    int status = 0;
#ifdef _POSIX_SAVED_IDS
    status = seteuid (euid);
#else
    status = setreuid (ruid, euid);
#endif

    if (status < 0) {
        fprintf(stderr, "Failed to set uid\n");
        exit(status);
    }
    return;
}

/* Set the effective UID to the real UID. */
void undo_setuid(void) {
    int status = 0;
#ifdef _POSIX_SAVED_IDS
    status = seteuid (ruid);
#else
    status = setreuid (euid, ruid);
#endif
    if (status < 0) {
        fprintf (stderr, "Couldn't set uid.\n");
        exit (status);
    }
    return;
}

int check_symlink(const char *file_name) {
    struct stat p_statbuf;

    if (lstat(file_name, &p_statbuf) < 0) {
        return FORUM_ERR;
    }
    if ((S_ISLNK(p_statbuf.st_mode)) == 1) {
        return FORUM_ERR;
    }

    return FORUM_OK;
}

int get_file_content(char *file_name, char *buffer) {
    int ret = 0;
    FILE *file = NULL;

    if (file_name == NULL || buffer == NULL) return FORUM_ERR;

    if ((check_symlink(file_name)) != FORUM_OK) return FORUM_ERR;

    /* Before file operations, remember the real UID and effective UID*/
    ruid = getuid ();
    euid = geteuid ();
    do_setuid ();
    if ((file = fopen(file_name, "rb")) == NULL) return FORUM_ERR;
    undo_setuid();

    if ((ret = fread(buffer, 1, MAX_BUFF_LEN, file)) < 1) {
        fclose(file);
        return FORUM_ERR;
    }

    fclose(file);

    return FORUM_OK;
}

int set_file_content(int size, char *prefix, char *name, char *path, char *content) {
    int offset = 0;
    FILE *file = NULL;
    char file_name[FILE_NAME_LEN + 1] = {0};

    if (name == NULL || path == NULL || content == NULL || prefix == NULL) {
        return FORUM_ERR;
    }

    ruid = getuid ();
    euid = geteuid ();
    do_setuid ();
    snprintf(file_name, sizeof(file_name), "%s%s", prefix, name);
    file = fopen(file_name, "wb");
    if (file == NULL) {
        return FORUM_ERR;
    }
    undo_setuid();

    do {
        int written = fwrite(content, 1, size - offset, file);
        if (written < 1) {
            fclose(file);
            return FORUM_ERR;
        }
        offset += written;
    } while (offset < size);

    fclose(file);
    strlcpy(path, file_name, FILE_NAME_LEN);
    return FORUM_OK;
}
