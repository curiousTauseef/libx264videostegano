#ifndef MESSAGE_H_
#define MESSAGE_H_
#include <stdlib.h>
#include <stdio.h>
#define true 1
#define false 0



struct _Message {
    char            *body;
    int             width;
    int             height;
    int             length;
    int             cursor;
    float           progress;
    size_t          frameNumber;
    int             gettingCapacity;
    int             isSendingMessage;
    size_t          numberOfEmbededBits;
    int             isInitialized;
};

typedef struct _Message stuMessage;


char msg_read_bit(stuMessage *_message);

#endif
