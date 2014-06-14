#ifndef __SERVER_H
#define __SERVER_H


#define MAXRECVLEN 500
#define PORTNUM 5000

/* server state*/
struct forum_server {
    int port; /* TCP listening port */
    char bindaddr; /* Bindding address */
};

/* function declaration */
void init_server();

#endif
