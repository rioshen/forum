#ifndef __FDB_H
#define __FDB_H

int init_database(void);
int authentication(char *username, char *password);
int add_post(char *name, char *content);
int add_file(char *name, char *path);
int add_account(char *name, char *password);
void get_post(char *buff);
void show_post(char *id, char *buffer);
void get_file(char *id, char *path);
void get_file_list(char *buffer);

#endif /* __FDB_H */
