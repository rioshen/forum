#ifndef __SERVER_H
#define __SERVER_H

/* server state*/
struct forum_server {
    int port; /* TCP listening port */
    char bindaddr; /* Bindding address */
};

/* function declaration */
void init_server();

#endif
