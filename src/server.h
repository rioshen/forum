#ifndef __SERVER_H
#define __SERVER_H


#define MAXRECVLEN 500
#define PORTNUM 6000

#define CMD_LOGIN   "LOGIN"
#define CMD_ADDPOST "ADDPOST"
#define CMD_DISPLAY "DISPLAY"
#define CMD_SHOW_POST "SHOWPOST"

#define OPT_SUCCESS "SUCCESS"
#define OPT_FAILED  "FAILED"




#define CMD_MAX_LEN     10
#define FIELD_ONE_LEN   128
#define FIELD_TWO_LEN   1024
#define CLIENT_MSG_LEN  (CMD_MAX_LEN + FIELD_ONE_LEN + FIELD_TWO_LEN + 3)


struct Action {
    char cmd[CMD_MAX_LEN + 1];
    char field1[FIELD_ONE_LEN + 1];
    char field2[FIELD_TWO_LEN + 1];
};


/* server state*/
struct forum_server {
    int port; /* TCP listening port */
    char bindaddr; /* Bindding address */
};

/* function declaration */
void init_server();
int read_sock(int sock, char *buf, size_t len);

#endif /* __SERVER_H */
