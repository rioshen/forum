#ifndef __SERVER_H
#define __SERVER_H


#define MAXRECVLEN 500
#define PORTNUM 5000

#define AUTH_START   "AUTH START"
#define AUTH_SUCCESS "AUTH SUCCESS"
#define AUTH_FAILED  "AUTH FAILED"

/* server state*/
struct forum_server {
    int port; /* TCP listening port */
    char bindaddr; /* Bindding address */
};

/* function declaration */
void init_server();

#endif /* __SERVER_H */
