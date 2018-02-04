#ifndef DECODEMESSAGE_H
#define DECODEMESSAGE_H
//typedef int bool;
#define true 1
#define false 0
#include <time.h>


struct _DecodeMessage {
    int messageSection;
    int cursor ;
    int messageLength;
    char* extension;
    int isBusy;
    char* recievedMessageBits;
    char* startBitsBoundary;
    float progress;
};
typedef struct _DecodeMessage stuDecodedMessage;

void decode_msg_add_bit(char bit, stuDecodedMessage *_decodedMessage);
stuDecodedMessage initDecodedMessage(void);
char bitsToByteChar(char* _byte);


#endif
