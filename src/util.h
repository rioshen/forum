#ifndef UTIL_H
#define UTIL_H

#define FORUM_OK  0XFFFFFE
#define FORUM_ERR 0XFFFFFF

#define MAX_CONTENT_LEN 1024
#define MAX_BUFF_LEN    1024
#define DEFAULT_ADDRESS "127.0.0.1"
#define DEFAULT_PORT    5000
#define ADDRESS_LEN     255
#define FILE_PREFIX "./upload/"
#define FILE_NAME_LEN strlen(FILE_PREFIX) + FIELD_ONE_LEN

int get_file_content(char *file_name, char *buffer);
int set_file_content(int size, char *prefix, char *name, char *path, char *content);

#endif /* UTIL_H */
