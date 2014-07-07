#ifndef __SERVER_H
#define __SERVER_H


#define MAXRECVLEN 500
#define PORTNUM 5000

#define CMD_SIGNUP    "SIGNUP"
#define CMD_LOGIN     "LOGIN"
#define CMD_ADDPOST   "ADDPOST"
#define CMD_DISPLAY   "DISPLAY"
#define CMD_SHOW_POST "SHOWPOST"
#define CMD_UPLOAD    "UPLOAD"
#define CMD_FILE      "FILE"
#define CMD_DOWNLOAD  "DOWNLOAD"

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

#endif /* __SERVER_H */
