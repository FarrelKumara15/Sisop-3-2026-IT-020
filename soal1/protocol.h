#ifndef PROTOCOL_H
#define PROTOCOL_H

#define MAX_MSG 256
#define MAX_NAME 50

typedef struct {
    char name[MAX_NAME];
    char msg[MAX_MSG];
} Message;

#endif
