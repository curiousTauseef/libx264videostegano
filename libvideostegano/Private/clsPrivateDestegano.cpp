#include "clsPrivateDestegano.h"
#include "clsPrivateStegano.h"
#include "Private/hlpFunctions.h"

clsPrivateDestegano::clsPrivateDestegano()
{
}

void binaryToAscii(const char* input, char* message)
{
    int length = strlen(input);     //get length of string
    int binary[8];    //array used to store 1 byte of binary number (1 character)
    int asciiNum = 0;      //the ascii number after conversion from binary
    char ascii;      //the ascii character itself


    int z = 0;   //counter used

    for(int x = 0; x < length / 8; x++)     //reading in bytes. total characters = length / 8
    {
        for(int a = 0; a < 8; a++)      //store info into binary[0] through binary[7]
        {
            binary[a] = (int) input[z] - 48;      //z never resets
            z++;
        }

        int power[8];    //will set powers of 2 in an array
        int counter = 7;        //power starts at 2^0, ends at 2^7
        for(int x = 0; x < 8; x++)
        {
            power[x] = counter;      //power[] = {7, 6, 5, ..... 1, 0}
            counter--;    //decrement counter each time
        }

        for(int y = 0; y < 8; y++)    //will compute asciiNum
        {
            double a = binary[y];    //store the element from binary[] as "a"
            double b = power[y];    //store the lement from power[] as "b"

            asciiNum += a* pow(2, b);   //asciiNum = sum of a * 2^power where 0 <= power <= 7, power is int
        }

        ascii = asciiNum;   //assign the asciiNum value to ascii, to change it into an actual character
        asciiNum = 0;    //reset asciiNum for next loop
        message[x] = ascii;
    }
}

void clsPrivateDestegano::initDecodedMessage()
{
    this->decodedMessage = new stuDecodedMessage();
    this->decodedMessage->messageSection = 0;
    this->decodedMessage->isBusy = true;
    this->decodedMessage->recievedMessageBits = (char*) malloc((START_BITS_COUNT + 1)*sizeof(char));
    this->decodedMessage->messageLength = 0;
    this->decodedMessage->cursor = 0;
    char* tmp = clsPrivateStegano::startBit();
    for(int i = 0; i < START_BITS_COUNT; ++i)
        tmp[i] += 48;
    this->decodedMessage->startBitsBoundary = (char*)malloc((START_BITS_COUNT + 1) * sizeof(char));
    memmove(this->decodedMessage->startBitsBoundary, tmp, START_BITS_COUNT * sizeof(char));
    this->decodedMessage->startBitsBoundary[START_BITS_COUNT] = '\0';
}

void clsPrivateDestegano::storeMessage(const char *_outputMessageFilePath)
{
    char* decodedCharMessage = (char*) malloc((1 + this->decodedMessage->messageLength / BYTE) * sizeof(char));
    binaryToAscii(this->decodedMessage->recievedMessageBits, decodedCharMessage);
    decodedCharMessage[this->decodedMessage->messageLength / BYTE] = '\0';
    std::string fileName;
    if(strlen(this->decodedMessage->extension) != 0)
        fileName = std::string(_outputMessageFilePath) + "." + std::string(this->decodedMessage->extension);
    else
        fileName = std::string(_outputMessageFilePath);
    FILE* pFile = fopen(fileName.c_str(), "wb");
    fwrite(decodedCharMessage, sizeof(char), this->decodedMessage->messageLength / BYTE, pFile);
    fclose(pFile);
    this->decodedMessage->isBusy = false;
}
