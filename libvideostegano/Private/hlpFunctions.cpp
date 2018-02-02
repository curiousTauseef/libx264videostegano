#include "hlpFunctions.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define START_BITS_COUNT 32
#define EXTENSION_BITS_COUNT 8
#define MESSAGE_SIZE_BITS_COUNT 32
off_t get_file_length( FILE *file ) {
    fpos_t position; // fpos_t may be a struct and store multibyte info
    off_t length; // off_t is integral type, perhaps long long

    fgetpos( file, &position ); // save previous position in file

    fseeko( file, 0, SEEK_END ); // seek to end
    length = ftello( file ); // determine offset of end

    fsetpos( file, &position ); // restore position

    return length;
}

const char *get_filename_extension(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

// \TODO check if bodyTmp is unnecessary
void messageInit(const char* _filepath, stuMessage *_message) {
    FILE *pFile = fopen(_filepath, "rb");
    size_t msgLength = get_file_length(pFile);
    const char *extension = get_filename_extension(_filepath);
    size_t extensionLength = strlen(extension);
    size_t rawMessagLength = EXTENSION_BITS_COUNT + extensionLength * BYTE + MESSAGE_SIZE_BITS_COUNT + msgLength;
    char *body = (char*)malloc((rawMessagLength + 1) * sizeof(char));
    body[rawMessagLength] = '\0';
    size_t read_number = fread(body + EXTENSION_BITS_COUNT + extensionLength * BYTE + MESSAGE_SIZE_BITS_COUNT, sizeof(char), msgLength, pFile);
    if(read_number != msgLength) {
        fprintf(stderr, "reading %s was not successful. Actual number of char read is: %d, but it should have been : %d\n", _filepath, read_number, msgLength);
        exit(1);
    }
    fclose(pFile);
    // put extension size in bit
    for(int i = 0; i < EXTENSION_BITS_COUNT; ++i) {
        body[i] = (extensionLength >> (EXTENSION_BITS_COUNT - i - 1))&1;
    }
    // put extension in bit
    if(extensionLength != 0) {
        char* extensionBit = (char*)malloc((extensionLength * BYTE + 1) * sizeof(char));
        ascii2Binary(extension, extensionBit, extensionLength);
        memcpy(body + EXTENSION_BITS_COUNT, extensionBit, extensionLength * BYTE);
    }

    // put message size in bit
    for(int i = 0; i < MESSAGE_SIZE_BITS_COUNT; ++i) {
        body[EXTENSION_BITS_COUNT + extensionLength * BYTE + i] = (msgLength >> (MESSAGE_SIZE_BITS_COUNT -  i - 1)) & 1;
    }
    // put message in bit
    size_t rawBitsMessageLength = EXTENSION_BITS_COUNT + extensionLength * BYTE + MESSAGE_SIZE_BITS_COUNT + msgLength * BYTE;
    char* bodyTmp = (char*)malloc((msgLength * BYTE + 1)*sizeof(char) );
    ascii2Binary(body + EXTENSION_BITS_COUNT + extensionLength * BYTE + MESSAGE_SIZE_BITS_COUNT, bodyTmp, msgLength);


    _message->cursor = 0;
    _message->numberOfEmbededBits = 0;
    _message->frameNumber = 0;
    _message->length = START_BITS_COUNT + rawBitsMessageLength;
    _message->progress = 1.0/_message->length;
    _message->body = (char*)malloc((_message->length + 1)* sizeof(char));
    _message->gettingCapacity = 0;

    memmove(_message->body + START_BITS_COUNT + EXTENSION_BITS_COUNT + extensionLength * BYTE + MESSAGE_SIZE_BITS_COUNT, bodyTmp, msgLength * BYTE);
    memmove(_message->body + START_BITS_COUNT, body, EXTENSION_BITS_COUNT + extensionLength * BYTE + MESSAGE_SIZE_BITS_COUNT);
    _message->body[_message->length] ='\0';
//    _message->isSendingMessage = true;
}

void ascii2Binary(const char* input, char* binary_array, int msgLength)
{
    int cursor = 0;
    int mask = 1;
    for(int x = 0; x < msgLength; x++)          //repeat until user's input have all been read
        // x < length because the last character is "\0"
    {

        // \TODO memcpy or memmv do it with more caution
        mask = 1;
        char* reverseBinary = (char*) malloc(BYTE);
        for (int cnt = 0; cnt < BYTE; ++cnt) {
            char newMask = mask << cnt;
            reverseBinary[cnt] = (newMask & input[x]) != 0;
        }
        for(int cnt = 0; cnt < BYTE; ++cnt) {
            binary_array[cursor++] = reverseBinary[BYTE-1-cnt];
        }
        free(reverseBinary);

    }
    binary_array[cursor] = '\0';

}
