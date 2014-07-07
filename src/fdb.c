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
#include <string.h>
#include <assert.h>

#include "util.h"
#include "../lib/sqlite3.h"

#define DB_NAME "forum.db"

#define ACCOUNT_TABLE       "ACCOUNT"
#define CREATE_ACCOUNT_STMT "id       INTEGER PRIMARY KEY AUTOINCREMENT," \
                            "name     TEXT              NOT NULL," \
                            "password TEXT            NOT NULL"
#define ADD_ACCOUNT_STMT    "INSERT INTO ACCOUNT (name, password) VALUES ('%q', '%q');"
#define QUERY_ACCOUNT_STMT  "SELECT * FROM ACCOUNT WHERE name = '%q' and password = '%q';"

#define POST_TABLE          "POST"
#define CREATE_POST_STMT    "id     INTEGER PRIMARY KEY AUTOINCREMENT," \
                            "name   TEXT            NOT NULL," \
                            "post   TEXT            NOT NULL"
#define ADD_POST_STMT        "INSERT INTO POST (name, post) VALUES ('%q', '%q');"
#define QUERY_POST_LIST      "SELECT id, name FROM POST;"
#define QUERY_POST_ENTRY     "SELECT post FROM POST WHERE id = '%q';"

#define FILE_TALBE           "UPLOAD"
#define CREATE_FILE_STMT     "id        INTEGER PRIMARY KEY AUTOINCREMENT," \
                             "name     TEXT              NOT NULL," \
                             "path     TEXT              NOT NULL"
#define ADD_FILE_STMT        "INSERT INTO UPLOAD (name, path) VALUES ('%q', '%q');"
#define QUERY_FILE_LIST      "SELECT id, name, path FROM UPLOAD;"
#define QUERY_FILE_STMT      "SELECT path FROM UPLOAD WHERE id = '%q';"

int is_valid = 0;
int g_status = 0;
char g_buffer[MAX_BUFF_LEN + 1] = {0};

/**
 * Creates a new table if it doesn't exit yet.
 * Returns sqlite3 error codes - in event of fail.
 * Returns FORUM_OK - success
 */
static int create_table(char *db_name, char *table, char *sql_stmt) {
    int rc = FORUM_OK;
    char *sql = NULL;
    char *err_msg = NULL;
    sqlite3 *database = NULL;

    /* Open database */
    if ((rc = sqlite3_open(db_name, &database)) != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(database));
        return rc;
    }

    /* Create a table if it doesn't exist. To protect SQL injection, use
     * built-in sqlite3_mprintf which needs to free memory after using. */
    sql = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS %s(%s);", table, sql_stmt);
    if ((rc = sqlite3_exec(database, sql, NULL, NULL, &err_msg)) != SQLITE_OK) {
        fprintf(stderr, "[%d]: Create table error: %s\n", rc, err_msg);
        sqlite3_free(err_msg);
        sqlite3_free(sql);
        (void)sqlite3_close(database);

        return rc;
    }

    sqlite3_free(err_msg);
    sqlite3_free(sql);

    /* Close the database */
    if ((rc = sqlite3_close(database)) != SQLITE_OK) {
        fprintf(stderr, "[%d]: Failed to close %s", rc, db_name);
        return rc;
    }

    return FORUM_OK;
}

int init_database(void) {
    int ret = FORUM_OK;

    ret = create_table(DB_NAME, ACCOUNT_TABLE, CREATE_ACCOUNT_STMT);
    if (ret != FORUM_OK) {
        fprintf(stderr, "[%d] Failed to create account table.", ret);
        return ret;
    }

    ret = create_table(DB_NAME, POST_TABLE, CREATE_POST_STMT);
    if (ret != FORUM_OK) {
        fprintf(stderr, "[%d] Failed to create post table.", ret);
        return ret;
    }

    ret = create_table(DB_NAME, FILE_TALBE, CREATE_FILE_STMT);
    if (ret != FORUM_OK) {
        fprintf(stderr, "[%d] Failed to create upload table.", ret);
        return ret;
    }

    return FORUM_OK;
}

/**
 * Inserts a new entry into the specified table.
 * Returns sqlite3 error code - in event of failed
 * Returns FORUM_OK - success
 */
static int add_value(char *db_name, char *sql_stmt) {
    char *err_msg = NULL;
    sqlite3 *database = NULL;
    int rc = SQLITE_OK;

    assert(db_name != NULL);
    assert(sql_stmt != NULL);

    /* Open database */
    if ((rc = sqlite3_open(db_name, &database)) != SQLITE_OK) {
        fprintf(stderr, "[%d]: Can't open database: %s\n", rc, sqlite3_errmsg(database));
        return rc;
    }

    /* Exectue sql statement */
    if ((rc = sqlite3_exec(database, sql_stmt, NULL, NULL, &err_msg)) != SQLITE_OK) {
        fprintf(stderr, "[%d]: Failed to insert an entry: %s\n", rc, err_msg);
        sqlite3_free(err_msg);
        (void)sqlite3_close(database);

        return rc;
    }

    sqlite3_free(err_msg);

    /* Close the database */
    if ((rc = sqlite3_close(database)) != SQLITE_OK) {
        fprintf(stderr, "[%d]: Failed to close %s", rc, db_name);
        return rc;
    }

    return FORUM_OK;
}

int add_account(char *name, char *password) {
    int ret = FORUM_OK;
    char *sql = NULL;

    assert(name != NULL);
    assert(password != NULL);

    sql = sqlite3_mprintf(ADD_ACCOUNT_STMT, name, password);
    ret = add_value(DB_NAME, sql);
    if (ret != FORUM_OK) {
        fprintf(stderr, "Failed to add user account into database.");
    }
    sqlite3_free(sql);

    return ret;
}

int add_post(char *name, char *content) {
    int ret = FORUM_OK;
    char *sql = NULL;

    assert(name != NULL);
    assert(content != NULL);

    sql = sqlite3_mprintf(ADD_POST_STMT, name, content);
    ret = add_value(DB_NAME, sql);
    if (ret != FORUM_OK) {
        fprintf(stderr, "Failed to add post into database.");
    }
    sqlite3_free(sql);

    return ret;
}

int add_file(char *name, char *path) {
    int ret = FORUM_OK;
    char *sql = NULL;

    if (name == NULL || path == NULL) {
        return FORUM_ERR;
    }

    sql = sqlite3_mprintf(ADD_FILE_STMT, name, path);
    ret = add_value(DB_NAME, sql);
    if (ret != FORUM_OK) {
        fprintf(stderr, "Failed to add post into database.");
    }
    sqlite3_free(sql);

    return FORUM_OK;
}

static int query_callback(void *data, int argc, char **argv, char **azColName) {
    int i = 0;
    char buffer[MAX_BUFF_LEN + 1] = {0};

    switch (g_status) {
        case 1:
            is_valid = 1;
            break;
        case 2:
            for(i = 0; i + 1 < argc; i = i + 2) {
                snprintf(buffer, sizeof(buffer), "%s: %s\n", argv[i], argv[i + 1]);
                if (strnlen(g_buffer, sizeof(g_buffer)) == 0) {
                    strlcpy(g_buffer, buffer, MAX_BUFF_LEN);
                } else {
                    strlcat(g_buffer, buffer, MAX_BUFF_LEN);
                }
            }
            break;
        case 3:
            strlcpy(g_buffer, argv[0], MAX_BUFF_LEN);
            break;
        case 4:
            strlcpy(g_buffer, argv[0], MAX_BUFF_LEN);
            break;
        case 5:
            for (i = 0; i + 2 < argc; i = i + 3) {
                snprintf(buffer, sizeof(buffer), "%s: %s: %s\n", argv[i], argv[i + 1], argv[i + 2]);
                if (strnlen(g_buffer, sizeof(g_buffer)) == 0) {
                    strlcpy(g_buffer, buffer, MAX_BUFF_LEN);
                } else {
                    strlcat(g_buffer, buffer, MAX_BUFF_LEN);
                }
            }
            break;
        default:
            break;
    }

    return 0;
}

/**
 * Fetch entry from specified table.
 * Returns sqlite3 error codes - in event of fail.
 * Returns FORUM_OK - success
 */
static int query(char *db_name, char *sql_stmt) {
    char *err_msg = NULL;
    sqlite3 *database = NULL;
    int rc = SQLITE_OK;
    const char* data = "Callback function called";

    assert(db_name != NULL);
    assert(sql_stmt != NULL);

    /* Open database */
    if ((rc = sqlite3_open(db_name, &database)) != SQLITE_OK) {
        fprintf(stderr, "[%d]: Can't open database: %s\n", rc, sqlite3_errmsg(database));
        return rc;
    }

    /* Execute sql statement */
    if ((rc = sqlite3_exec(database, sql_stmt, query_callback, (void *)data, &err_msg)) != SQLITE_OK) {
        fprintf(stderr, "[%d]: Failed to retrieval an entry: %s\n", rc, err_msg);
        sqlite3_free(err_msg);
        (void)sqlite3_close(database);

        return rc;
    }

    sqlite3_free(err_msg);

    /* Close the database */
    if ((rc = sqlite3_close(database)) != SQLITE_OK) {
        fprintf(stderr, "[%d]: Failed to close %s", rc, db_name);
        return rc;
    }

    return FORUM_OK;
}

void get_file(char *id, char *path) {
    int rc = SQLITE_OK;
    char *sql = NULL;

    if (id == NULL || path == NULL) {
        return;
    }

    g_status = 4;

    sql = sqlite3_mprintf(QUERY_FILE_STMT, id);
    if ((rc = query(DB_NAME, sql)) != FORUM_OK) {
        fprintf(stderr, "[%d]: Failed to query", rc);
        return;
    }

    strlcpy(path, g_buffer, MAX_BUFF_LEN);
    printf("Get file %s\n", path);
    memset(g_buffer, 0, sizeof(g_buffer));

    return;
}

void get_file_list(char *buffer) {
    int rc = SQLITE_OK;
    char *sql = NULL;

    g_status = 5;

    sql = sqlite3_mprintf(QUERY_FILE_LIST);
    if ((rc = query(DB_NAME, sql)) != FORUM_OK) {
        fprintf(stderr, "[%d]: Failed to query", rc);
        return;
    }

    strlcpy(buffer, g_buffer, MAX_BUFF_LEN);
    printf("%s\n", buffer);
    memset(g_buffer, 0, sizeof(g_buffer));

    return;
}

void get_post(char *buff) {
    int rc = SQLITE_OK;
    char *sql = NULL;

    g_status = 2;

    sql = sqlite3_mprintf(QUERY_POST_LIST);
    if ((rc = query(DB_NAME, sql)) != FORUM_OK) {
        fprintf(stderr, "[%d]: Failed to query", rc);
        return;
    }

    strlcpy(buff, g_buffer, MAX_BUFF_LEN);
    memset(g_buffer, 0, sizeof(g_buffer));

    return;
}

void show_post(char *id, char *buffer) {
    int rc = SQLITE_OK;
    char *sql = NULL;

    g_status = 3;

    sql = sqlite3_mprintf(QUERY_POST_ENTRY, id);
    if ((rc = query(DB_NAME, sql)) != FORUM_OK) {
        fprintf(stderr, "[%d]: Failed to query", rc);
        return;
    }
    if (strlen(g_buffer) == 0) {
        strlcpy(buffer, "Invalid ID", sizeof("Invalid ID"));
    } else {
        strlcpy(buffer, g_buffer, MAX_BUFF_LEN);
    }
    memset(g_buffer, 0, sizeof(g_buffer));
    return;
}

int authentication(char *username, char *password) {
    int rc = SQLITE_OK;
    char *sql = NULL;

    assert(username != NULL);
    assert(password != NULL);

    g_status = 1;

    sql = sqlite3_mprintf(QUERY_ACCOUNT_STMT, username, password);
    if ((rc = query(DB_NAME, sql)) != FORUM_OK) {
        fprintf(stderr, "[%d]: Failed to query", rc);
        return FORUM_ERR;
    }

    if (is_valid != 1) {
        return FORUM_ERR;
    } else {
        is_valid = 0;
        return FORUM_OK;
    }
}
