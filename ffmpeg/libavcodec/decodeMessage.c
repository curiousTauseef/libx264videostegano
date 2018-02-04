#include "decodeMessage.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BYTE 8
#define START_BITS_COUNT 32
#define EXTENSION_BITS_COUNT 8
#define MESSAGE_SIZE_BITS_COUNT 32

char bitsToByteChar(char* _byte)
{
    char returnValue = _byte[0] << 7 | _byte[1] << 6 | _byte[2] << 5 | _byte[3] << 4 | _byte[4] << 3 | _byte[5] << 2 | _byte[6] << 1 | _byte[7];
    return returnValue;
}

void decode_msg_add_bit(char bit, stuDecodedMessage *_decodedMessage)
{
//    printf("char: %d\n",bit);
    if(_decodedMessage == NULL || _decodedMessage->isBusy == false)
        return;
//    FILE* pFile = fopen("ffmpeg.log", "a");
//    fprintf(pFile,"%d",bit);/
//    fclose(pFile);
    if(_decodedMessage->messageSection == 0) { // getting start bit
        if(_decodedMessage->cursor != START_BITS_COUNT) {
            _decodedMessage->recievedMessageBits[_decodedMessage->cursor++] = bit + 48;
            return;
        }
        _decodedMessage->recievedMessageBits[START_BITS_COUNT] = '\0';
        if(strncmp(_decodedMessage->recievedMessageBits, _decodedMessage->startBitsBoundary, START_BITS_COUNT) == 0) {
            printf("Start bit detected\n");
            printf("StartBitBoundary: %s\n",_decodedMessage->startBitsBoundary);
            printf("recieved message: %s\n",_decodedMessage->recievedMessageBits);
            _decodedMessage->messageSection++; // getting 8 bits of extension size
            _decodedMessage->recievedMessageBits[EXTENSION_BITS_COUNT] = '\0';
            _decodedMessage->cursor = 0;
            _decodedMessage->recievedMessageBits[_decodedMessage->cursor++] = bit;
            return;
        }
        else {
            memmove(_decodedMessage->recievedMessageBits, _decodedMessage->recievedMessageBits + 1, START_BITS_COUNT-1);
            _decodedMessage->recievedMessageBits[START_BITS_COUNT-1] = bit + 48;
            _decodedMessage->recievedMessageBits[START_BITS_COUNT] = '\0';
            return;
        }
    } else if (_decodedMessage->messageSection == 1) { // getting 8 bits of extension size
        if(_decodedMessage->cursor < EXTENSION_BITS_COUNT)
            _decodedMessage->recievedMessageBits[_decodedMessage->cursor++] = bit;
        else {
            int extensionLength = 0;
            for(int i = 0; i < EXTENSION_BITS_COUNT; ++i) {
                extensionLength += _decodedMessage->recievedMessageBits[i]*pow(2, EXTENSION_BITS_COUNT - 1 - i);
            }
            _decodedMessage->messageSection++; // getting extensionLength of 8 of bits for char of extension
            if(extensionLength != 0) {
            _decodedMessage->messageLength = extensionLength * BYTE;
            _decodedMessage->recievedMessageBits = (char*)malloc((_decodedMessage->messageLength + 1) * sizeof(char));
            _decodedMessage->recievedMessageBits[_decodedMessage->messageLength] = '\0';
            } else { // extension size is zero
                _decodedMessage->messageSection++; // getting message size, lets get message size
                _decodedMessage->extension = (char*)malloc((1) * sizeof(char));
                _decodedMessage->extension[0] = '\0';
                _decodedMessage->recievedMessageBits[MESSAGE_SIZE_BITS_COUNT] = '\0';
            }
            _decodedMessage->cursor = 0;
            _decodedMessage->recievedMessageBits[_decodedMessage->cursor++] = bit;

        }
    } else if(_decodedMessage->messageSection == 2) { // getting extensionLength of char of extension
        if(_decodedMessage->cursor < _decodedMessage->messageLength)
            _decodedMessage->recievedMessageBits[_decodedMessage->cursor++] = bit;
        else {
            _decodedMessage->extension = (char*)malloc((_decodedMessage->messageLength/BYTE + 1) * sizeof(char));
            _decodedMessage->extension[_decodedMessage->messageLength/BYTE] = '\0';
            for(int i = 0; i < _decodedMessage->messageLength/BYTE; ++i) {
                _decodedMessage->extension[i] = bitsToByteChar(_decodedMessage->recievedMessageBits  + i*BYTE);
            }
            _decodedMessage->messageSection++; // getting message size
            _decodedMessage->cursor = 0;
            _decodedMessage->recievedMessageBits[MESSAGE_SIZE_BITS_COUNT] = '\0';
            _decodedMessage->recievedMessageBits[_decodedMessage->cursor++] = bit;
        }
    } else if(_decodedMessage->messageSection == 3) { // getting message length
        if(_decodedMessage->cursor < MESSAGE_SIZE_BITS_COUNT)
            _decodedMessage->recievedMessageBits[_decodedMessage->cursor++] = bit;
        else {
            _decodedMessage->messageLength = 0;
            for(int i = 0; i < MESSAGE_SIZE_BITS_COUNT; ++i) {
                _decodedMessage->messageLength += _decodedMessage->recievedMessageBits[i]*pow(2, MESSAGE_SIZE_BITS_COUNT - 1 - i);
            }
            _decodedMessage->messageLength *= BYTE;
            _decodedMessage->messageSection++; // getting plain message
            _decodedMessage->cursor = 0;
            _decodedMessage->recievedMessageBits = (char*)malloc((_decodedMessage->messageLength + 1) * sizeof(char));
            _decodedMessage->recievedMessageBits[_decodedMessage->cursor++] = bit + 48;
        }
    } else if(_decodedMessage->messageSection == 4) { // getting plain message
        _decodedMessage->progress += 1.0 / _decodedMessage->messageLength;
        if(_decodedMessage->cursor < _decodedMessage->messageLength - 1)
            _decodedMessage->recievedMessageBits[_decodedMessage->cursor++] = bit + 48;
        else {
            _decodedMessage->recievedMessageBits[_decodedMessage->cursor++] = bit + 48;
            _decodedMessage->isBusy = false;
        }
    }
}


