A Secure Forum
===============

This is a secure project of my _Application Security_ course.

###Introduction
This project is implemented in `C`, in order to improve my secure design mindsets.

###Vulnerabilities and Protections
The purpose of this project is to examine my _Application Security_ mindsets. So here I will demonstrate what I have done to protect my program.

####C vulnerabilities
Most vulnerabilities in C are related to buffer overflows external link and string manipulation. In most cases, this would result in a segmentation fault, but specially crafted malicious input values, adapted to the architecture and environment could yield to arbitrary code execution.

#####1. Use `fgets()` instead of `gets()`.
The stdio `gets()` function does not check for buffer length and always results in a vulnerability.
```
printf("password:> ");
if (fgets(password, PASSWORD_LEN, stdin) == NULL) {
    fprintf(stderr, "Failed to get password");
    continue;
}
```

#####2. `strlcpy(), strlcat(), strncmp()` instead of `strcpy(), strcat(), strcmp() or strncpy(), strncat()`.
The `strcpy()`  built-in function does not check buffer lengths and may very well overwrite memory zone contiguous to the intended destination.
```
src/server.c:
if ((strncmp(action->cmd, CMD_SIGNUP, strlen(CMD_SIGNUP))) == 0) {

src/forum.c:
strlcpy(action.field1, command, strnlen(command, COMMAND_LEN));
```

#####3. String formatting attack.
Another important vulnerability is _string formatting attack_ which may cause information leakage, overwriting of memory, … This error can be exploited in any of the following functions: `printf, fprintf, sprintf` and `snprintf`.

To avoid this, most of my program is hard code the information while interacting with users. What's more, at least, I never use any user's input.

```
src/form.c:

fprintf(stderr, "Failed to get user name.\n");
continue;
```

####SQL Injection
Because I use `SQLite` as the back end database to store information. It is a must to considerate `SQL` injection. SQL injection attacks are very common due to two facts:
+ The significant prevalence of SQL Injection vulnerabilities.
+ The attractiveness of the target (i.e., the database typically contains all the interesting/critical data for your application).

To avoid this, my considerations contains:
+ Escaping all user supplied input.
+ Use of `prepared statement` queries

#####1. Validation of user input.
To escape and validate user input before putting it in query, I restrict that user input could only contain alphabetics and digits. Do input validation as the first step during the interaction.

```
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
```

And here's a typical usage:
```
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
```

#####2. Use of `prepared statement` queries
SQLite provides two ways to protect SQL injection:
+ `sqlite3_exec` and `sqlite3_mprintf` as an all-in-one interface to pre-compile SQL statement.
+ Use `%q` instead of `%s` to protect SQL injection.

For formatted string, SQLite provides `sqlite3_mprintf` and `%q` to do escape special characters from a formatted query before interacting with the database.

```
src/fdb.c

#define ADD_ACCOUNT_STMT    "INSERT INTO ACCOUNT (name, password) VALUES ('%q', '%q');"

sql = sqlite3_mprintf(ADD_ACCOUNT_STMT, name, password);
ret = add_value(DB_NAME, sql);
if (ret != FORUM_OK) {
    fprintf(stderr, "Failed to add user account into database.");
}
sqlite3_free(sql);
```

What's more, `sqlite3_exec` is a one-step query execute interface which is a convenience wrapper around `sqlite3_prepare_v2()`, `sqlite3_step()`, and `sqlite3_finalize()`, that allows an application to run multiple statements of SQL without having to use a lot of C code.

```
src/fdb.c:

/* Execute sql statement */
if ((rc = sqlite3_exec(database, sql_stmt, query_callback, (void *)data, &err_msg)) != SQLITE_OK) {
    fprintf(stderr, "[%d]: Failed to retrieval an entry: %s\n", rc, err_msg);
    sqlite3_free(err_msg);
    (void)sqlite3_close(database);

    return rc;
}
```

####File Vulnerabilities
Considerations of file operations contains:
+ Execute files by changing the UID.
+ Execute a symbolic link file which means leak some important information such as `/etc/passwd`

#####1. `setuid()` to protect privileges
Accessing a file usually indicates a security flaw.  If an attacker can change anything along the path between the call to access() and the file's actual use (e.g., by moving files), the attacker can exploit the race condition.

To avoid this, I choose to use `setuid()` to set up the correct permissions while opening file directly and cover it back after
accessing a file.

```
src/util.c:

/* Before file operations, remember the real UID and effective UID*/
ruid = getuid ();
euid = geteuid ();
do_setuid ();
if ((file = fopen(file_name, "rb")) == NULL) return FORUM_ERR;
undo_setuid();
```
#####2. Validate symbolic file.
To avoid symbolic file vulnerability (e.g, an attacker create a symbolic link from his own file to the `/etc/passwd` authentication file then opens the file for writing as root). Before uploading files to server, the program will check whether it is a symbolic file.

```
src/util.c:
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
```
