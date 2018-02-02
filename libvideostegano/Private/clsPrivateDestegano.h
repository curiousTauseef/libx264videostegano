#ifndef CLSDESTEGANO_H
#define CLSDESTEGANO_H

#include "clsAvMediaDecoder.h"
#include "clsDemuxer.h"
#include "SteganoCommon.h"

extern "C" {
#include <libavcodec/decodeMessage.h>
}

#define START_BITS_COUNT 32
/**
 * @brief The clsPrivateDestegano class is for managing private data of Destagno class
 */
class clsPrivateDestegano
{
public:
    /**
     * @brief clsPrivateDestegano
     */
    clsPrivateDestegano();
    /**
     * @brief initDecodedMessage initalize docoded message
     */
    void initDecodedMessage();
    /**
     * @brief storeMessage
     * @param _outputMessageFilePath
     */
    void storeMessage(const char *_outputMessageFilePath);

public:
    stuVideoInfo        info;
    clsDemuxer          demuxer;
    clsAvMediaDecoder   avMediaDecoder;
    stuDecodedMessage   *decodedMessage;
};

#endif
