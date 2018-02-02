#ifndef X264ENCODER_H
#define X264ENCODER_H

#ifdef __cplusplus
#define __STDINT_MACROS
#define __STDC_CONSTANT_MACROS
#endif
#include <iostream>
#include <queue>
#include <stdint.h>

#include <fstream>

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

extern "C" {
#include "x264.h"
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include "message.h"
}

class Stegano;
/**
 * @brief The clsX264Encoder class is aimed for encoding video in H.264
 */
class clsX264Encoder
{
public:
    /**
     * @brief clsX264Encoder default constructor
     */
    clsX264Encoder();
    ~clsX264Encoder();
    /**
     * @brief initialize all varaibles of encoding
     * @param _width
     * @param _height
     * @param _encodingProrfileIndex
     * @return
     */
    bool init(int _width, int _height, uint _fps_num, uint _fps_denum, int _encodingProrfileIndex = 0);
    bool reinit();
    /**
     * @brief encodeFrame
     * @param rgb_buffer
     * @return
     */
    AVPacket& encodeFrame(unsigned char *rgb_buffer);
    /**
     * @brief encode same as encodeFrame. (Deprecated)
     * @param rgb_buffer
     */
    void encode(unsigned char *rgb_buffer);
    /**
     * @brief isNalsAvailableInOutputQueue
     * @return
     */
    bool isNalsAvailableInOutputQueue();
    /**
     * @brief getNalUnit
     * @return
     */
    x264_nal_t getNalUnit();
    /**
     * @brief isInitialized
     * @return
     */
    bool isInitialized() const {return this->initialized;}
    /**
     * @brief embed
     * @param _filePath
     * @return
     */
    bool embed(std::string _filePath);

    int getPpsFrameLength() const {return this->ppsFrameLength;}
    int getSpsFrameLength() const {return this->spsFrameLength;}

    unsigned char *getPpsFrame() const {return this->ppsFrame;}
    unsigned char *getSpsFrame() const {return this->spsFrame;}

    AVPacket getAvpacket() {return this->avPacket;}

private:
    // Use this context to convert your BGR Image to YUV image since x264 do not support RGB input
    SwsContext* convertContext;
    std::queue<x264_nal_t> output_queue;
    x264_param_t parameters;
    x264_picture_t picture_in, picture_out;
    x264_t* encoder;
    AVPacket avPacket;
    int64_t frameNumbe = 0;
    stuMessage *message;
    bool initialized;
    int profileIndex;
    std::vector<std::string> encodingProfiles;

    unsigned char* ppsFrame = NULL;
    int ppsFrameLength = 0;
    unsigned char* spsFrame = NULL;
    int spsFrameLength = 0;

    friend class Stegano;

};

#endif // X264ENCODER_H
