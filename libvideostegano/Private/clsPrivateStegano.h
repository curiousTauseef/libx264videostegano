#ifndef CLSPRIVATESTEGANO
#define CLSPRIVATESTEGANO

#include "clsMuxer.h"
#include "clsDemuxer.h"
#include "clsX264encoder.h"
#include "clsAvMediaDecoder.h"
#include "Stegano.h"

extern "C" {
#include "message.h"
#include <libavcodec/decodeMessage.h>
}

#define START_BITS_COUNT 32
/**
 * @brief The clsPrivateStegano class is aimed for managing private data of Stegano class
 */
class clsPrivateStegano
{
public:
    /**
     * @brief fileExits
     * @param _filePath
     * @return
     */
    static bool fileExits(const std::string &_filePath);
    /**
     * @brief startBit create a random start bit for embedding using provided key file
     * @return
     */
    static char *startBit();
    /**
     * @brief isInitialized
     * @return
     */
    inline bool isInitialized() const {return this->initialized;}

public:
    stuVideoInfo        info;
    clsMuxer            muxer;
    clsDemuxer          demuxer;
    stuMessage          **message;
    bool                initialized;
    clsX264Encoder      x264Encoder;
    unsigned char       *decodedFrame;
    std::string         videoFilePath;
    clsAvMediaDecoder   avMediaDecoder;
    uint                maximumFrameNumber;
};

#endif
