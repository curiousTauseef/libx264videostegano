#include <unistd.h>

#include "clsVideoFramedSource.h"


EventTriggerId clsVideoFramedSource::eventTriggerId = 0;
unsigned clsVideoFramedSource::referenceCount = 0;
clsDemuxer *clsVideoFramedSource::demuxer = NULL;
clsX264Encoder *clsVideoFramedSource::encoder = NULL;
clsAvMediaDecoder *clsVideoFramedSource::decoder = NULL;
clsMuxer *clsVideoFramedSource::muxer = NULL;
stuMessage *clsVideoFramedSource::message = NULL;

clsVideoFramedSource *clsVideoFramedSource::create(UsageEnvironment &_usageEnvironment)
{
    return new clsVideoFramedSource(_usageEnvironment);
}


clsVideoFramedSource::clsVideoFramedSource(UsageEnvironment &_usageEnvironment) : FramedSource(_usageEnvironment)
{

    uint numberOfChannel = 3;
    decodedFrame = (unsigned char*)malloc((this->demuxer->getHeight() * this->demuxer->getWidth() * numberOfChannel));
    ++clsVideoFramedSource::referenceCount;
    this->frameNumber = 0;
    if(this->eventTriggerId == 0) {
        this->eventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
    }
}

clsVideoFramedSource::~clsVideoFramedSource()
{

    std::cout << "clsVideoFramedSource::~clsVideoFramedSource()" << std::endl;
    --clsVideoFramedSource::referenceCount;
    envir().taskScheduler().deleteEventTrigger(this->eventTriggerId);
    this->eventTriggerId = 0;
}
#define SYNCING_FRAMES 50
void clsVideoFramedSource::doGetNextFrame()
{
    this->frameNumber++;
    uint numberOfChannel = 3;

    bool playing = false;
    // \NOTE memory leak vulnerabilty! right now we have to call this->muxer->writeFrame(avPacket); for escaping from avpacket memry leak
    if(this->encoder->isNalsAvailableInOutputQueue()) {
        this->nalUnit = this->encoder->getNalUnit();
    } else if(this->frameNumber < SYNCING_FRAMES) {
        memset(decodedFrame, 255, (this->demuxer->getHeight() * this->demuxer->getWidth() * numberOfChannel));
        this->encoder->encodeFrame(decodedFrame);
        usleep(SLEEP_BETWEEN_FRAMES);
        this->nalUnit = this->encoder->getNalUnit();
    } else {
        if(this->frameNumber == SYNCING_FRAMES) {
            std::cout << "starting to send video" << std::endl;
            this->demuxer->reinit();
            this->decoder->init(this->demuxer->getAvFormatContext());
            this->muxer->init("notSupported.mp4", this->demuxer->getAvFormatContext(), true);
            this->encoder->init(25, 1, this->demuxer->getWidth(), this->demuxer->getHeight());
            this->message->isSendingMessage = true;
        }
        while(true) {
            playing = demuxer->getNextAvPacketFrame(avPacket);
            if(playing == false)
                break;
            if(avPacket.stream_index == demuxer->getVideoStreamIndex())
                break;
        }
        if(playing)
            this->decoder->decodeFrame(avPacket, decodedFrame);
    }
    if(playing) {
        AVPacket tmpPacket = this->encoder->encodeFrame(decodedFrame);
        AVPacket *ptmpPacket = &tmpPacket;
        av_packet_unref(ptmpPacket);
        memset(ptmpPacket, 0, sizeof(*ptmpPacket));
        av_init_packet(ptmpPacket);
        ptmpPacket = NULL;
        avPacket.data = tmpPacket.data;
        avPacket.size = tmpPacket.size;
        this->muxer->writeFrame(avPacket);
        gettimeofday(&currentTime, NULL);
        this->nalUnit = this->encoder->getNalUnit();
        usleep(SLEEP_BETWEEN_FRAMES);
    }
    if(this->nalUnit.i_payload == 0) {
        this->muxer->close(this->encoder->getPpsFrameLength(), this->encoder->getSpsFrameLength(),
                           this->encoder->getPpsFrame(), this->encoder->getSpsFrame());
        std::cerr << "Video is Done or what?" << std::endl;
        return;
    }
    deliverFrame();
}

void clsVideoFramedSource::deliverFrame0(void *clientData)
{
    ((clsVideoFramedSource*)clientData)->deliverFrame();
    std::cerr << "Why we see this??" << std ::endl;

}

void clsVideoFramedSource::deliverFrame()
{
    if(!isCurrentlyAwaitingData()) return;
    int trancate = 0;
    if (this->nalUnit.i_payload >= 4 && this->nalUnit.p_payload[0] == 0 && this->nalUnit.p_payload[1] == 0 && this->nalUnit.p_payload[2] == 0 && this->nalUnit.p_payload[3] == 1 ) {
        trancate = 4;
    }
    else {
        if(this->nalUnit.i_payload >= 3 && this->nalUnit.p_payload[0] == 0 && this->nalUnit.p_payload[1] == 0 && this->nalUnit.p_payload[2] == 1 ) {
            trancate = 3;
        }
    }
    if(this->nalUnit.i_payload - trancate > fMaxSize) {
        fFrameSize = fMaxSize;
        fNumTruncatedBytes = this->nalUnit.i_payload - trancate - fMaxSize;
    }
    else {
        fFrameSize = this->nalUnit.i_payload - trancate;
    }
    fPresentationTime = currentTime;
    memmove(fTo, this->nalUnit.p_payload + trancate, fFrameSize);
    this->nalUnit.i_payload = 0;
    FramedSource::afterGetting(this);


}

