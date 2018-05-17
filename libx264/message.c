#include "message.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* Message structure:
 * start-bits (4 bytes)
 * extensionSize (3 bits)
 * extension
 * msgSize (4 bytes)
 * msg
 *
 */




char messageReadBit(stuMessage *_message) {
    if(_message){
        _message->numberOfEmbededBits++;
        if(_message->gettingCapacity)
            return rand() % 2;
        if(_message->isSendingMessage == false)
            return 2;
        if(_message->cursor < _message->length) {
            _message->progress += 1.0/_message->length;
            return _message->body[_message->cursor++];
        }
        else {
            printf("A message with length %d bits is sent after %d frame. Frame size is: %d %d\n",
                   _message->length, _message->frameNumber, _message->width, _message->height);
            _message->isSendingMessage = false;
            _message->isInitialized=0;
            return 2;
        }
    } else
        return 2;
}
