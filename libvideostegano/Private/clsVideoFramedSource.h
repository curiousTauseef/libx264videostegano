#ifndef CLSVIDEOFRAMEDSOURCE_H
#define CLSVIDEOFRAMEDSOURCE_H

#include <iostream>
#include <FramedSource.hh>
#include <queue>
#include "clsDemuxer.h"
#include "clsMuxer.h"
#include "clsAvMediaDecoder.h"
#include "clsX264encoder.h"

extern "C" {
#include <message.h>
}


#define SLEEP_BETWEEN_FRAMES 3000

class clsVideoFramedSource : public FramedSource
{
public:
    static clsVideoFramedSource* create(UsageEnvironment &_usageEnvironment);
    static EventTriggerId eventTriggerId;

private:
    clsVideoFramedSource(UsageEnvironment &_usageEnvironment);
    virtual ~clsVideoFramedSource();

    virtual void doGetNextFrame();
    static void deliverFrame0(void *clientData);
    void deliverFrame();

private:
    static unsigned referenceCount;
    static bool firstTime;
    timeval currentTime;
    static clsDemuxer *demuxer;
    static clsAvMediaDecoder *decoder;
    static clsX264Encoder *encoder;
//    static clsMuxer *muxer;
    static stuMessage **message;
    x264_nal_t nalUnit;
    uint frameNumber;
    unsigned char* decodedFrame;
    AVPacket avPacket;

    friend class Stegano;




};

#endif // CLSVIDEOFRAMEDSOURCE_H
