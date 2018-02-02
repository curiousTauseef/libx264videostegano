#include "clsPrivateStegano.h"

bool clsPrivateStegano::fileExits(const std::string &_filePath)
{
    std::ifstream f(_filePath.c_str());
    return f.good();
}

char *clsPrivateStegano::startBit()
{
    srand(Stegano::key);
//    srand(22);
    char * startBitsChar = (char*) malloc((START_BITS_COUNT + 1)*sizeof(char));
    int i = 0;
    for (; i < START_BITS_COUNT; ++i)
        startBitsChar[i] = rand() % 2;
    startBitsChar[i] = '\n';
    return startBitsChar;
}
