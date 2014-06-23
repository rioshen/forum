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
#include "../lib/sqlite3.h"

#include "util.h"

#define DB_NAME "forum.db"

#define ACCOUNT_TABLE "ACCOUNT"
#define CREATE_ACCOUNT_STMT  "ID     INT PRIMARY KEY NOT NULL," \
                             "NAME   TEXT            NOT NULL," \
                             "PASSWD TEXT            NOT NULL"


#define POST_TABLE "POST"
#define CREATE_POST_STMT  "ID     INT PRIMARY KEY NOT NULL," \
                          "NAME   TEXT            NOT NULL," \
                          "POST   TEXT            NOT NULL"

/**
 * Creates a new table if it doesn't exit yet.
 * Returns sqlite3 error codes - in event of fail.
 * Returns FORUM_OK - sucess
 */
static int create_table(char *db_name, char *table, char *sql_stmt) {
    int rc = FORUM_OK;
    char *sql = NULL;
    char *zErrMsg = NULL;
    sqlite3 *database = NULL;

    /* Open database */
    if ((rc = sqlite3_open(db_name, &database)) != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(database));
        return rc;
    }

    /* Create a table if it doesn't exist. To protect SQL injection, use
     * built-in sqlite3_mprintf which needs to free memory after using. */
    sql = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS %s(%s);", table, sql_stmt);
    if ((rc = sqlite3_exec(database, sql, NULL, NULL, &zErrMsg)) != SQLITE_OK) {
        fprintf(stderr, "Create table error: %s\n", zErrMsg);
        fprintf(stderr, "Error no is %d\n", rc);
        sqlite3_free(zErrMsg);
        sqlite3_free(sql);
        (void)sqlite3_close(database);

        return rc;
    }

    sqlite3_free(zErrMsg);
    sqlite3_free(sql);

    /* Close the database */
    if ((rc = sqlite3_close(database)) != SQLITE_OK) {
        fprintf(stderr, "Failed to close %s", db_name);
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

    return FORUM_OK;
}

static int add_value(char *db_name, )

int main() {
    (void)init_database();
}

/**
int add_post(char* username, char* content) {
    int ret = FORUM_OK;
    char *sql = NULL;
    char *buffer = NULL;

    if (username == NULL || content == NULL) {
        assert(username != NULL);
        assert(content != NULL);
        return FORUM_ERR;
    }

    if (forum_db = NULL) {
        fprintf(stderr, "Database is not ready.\n");
        return FORUM_ERR;
    }

    buffer = (char *)malloc(strlen(content) + 1);
    memset(buffer, 0, strlen(content) + 1);

    sql = snprintf(sql,
        "INSERT INTO POST (NAME, POST) VALUES (%s, %s);",
        username, buffer)


}


int authenticate(char* username, char* password) {
    int ret = 0;

    if (username == NULL || password || NULL) {
        assert(username != NULL);
        assert(password != NULL);
        return FORUM_ERR;
    }
}
*/
