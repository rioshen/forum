A Secure Forum
===============

This is a secure project of my _Application Security_ course.

###Introduction
This project is implemented in `C`, in order to improve my secure design mindsets.

###Vulnerabilities and Protections
The purpose of this project is to examine my _Application Security_ mindsets. So here I will demonstrate what I have done to protect my program.

####SQL Injection
Because I use `SQLite` as the back end database to store information. It is a must to considerate `SQL` injection. SQL injection attacks are very common due to two facts:
+ The significant prevalence of SQL Injection vulnerabilities.
+ The attractiveness of the target (i.e., the database typically contains all the interesting/critical data for your application).

To avoid this, my considerations contains:
+ Escaping all user supplied input.
+ Use of `prepared statement` queries

#####Validation of user input.
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

####File Vulnerabilities
Considerations of file operations contains:
+ Execute files by changing the UID.
+ Execute a symbolic link file which means leak some important information such as `/etc/passwd`

#####`setuid()` to protect privileges
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
#####Validate `symbolic file`.
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
