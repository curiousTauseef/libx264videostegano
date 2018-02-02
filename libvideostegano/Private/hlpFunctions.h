#ifndef HLPFUNCTIONS_H
#define HLPFUNCTIONS_H

#define BYTE 8
extern "C" {
#include "message.h"
}
#include <stdlib.h>
#include <stdio.h>

void messageInit(const char* body, stuMessage *_message);
void ascii2Binary(const char* input, char* binary_array, int msgLength);
off_t get_file_length( FILE *file );
const char *get_filename_extension(const char *filename);


#endif
