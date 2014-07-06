#ifndef __FDB_H
#define __FDB_H

int init_database(void);
int authentication(char *username, char *password);
int add_post(char *name, char *content);
#endif /* __FDB_H */
