#include "Destegano.h"
#include "Stegano.h"
#include "Private/clsPrivateDestegano.h"
#include <stdlib.h>

#include "Private/ErrorStructure.hpp"

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavutil/imgutils.h>
#include<libavutil/opt.h>
#include <libavformat/avformat.h>
}


Destegano::Destegano() :
    pPrivate(new clsPrivateDestegano)
{
    av_register_all();
    avcodec_register_all();
    avformat_network_init();
    avdevice_register_all();

}

int Destegano::extractMessage(const std::string &_videoFilePath, const char *_outputMessageFilePath, float **_progress)
{

    if(this->pPrivate->demuxer.init(_videoFilePath.c_str()) == false) {
        int ret = 1;
        errorStringVector[ret] = "Could not open " + _videoFilePath;
        std::cerr << errorStringVector[ret] << std::endl;

        return ret;
    }
    this->pPrivate->initDecodedMessage();
    this->pPrivate->avMediaDecoder.decodedMessage = this->pPrivate->decodedMessage;
    bool extractedInfo = false;
    this->pPrivate->avMediaDecoder.init(this->pPrivate->demuxer.getAvFormatContext());
//    this->pPrivate->avMediaDecoder.init(this->pPrivate->demuxer.getWidth(), this->pPrivate->demuxer.getHeight(), AV_CODEC_ID_H264);
    AVFormatContext *inputVideoFormatContext = this->pPrivate->demuxer.getAvFormatContext();
    int videoStreamIndex = this->pPrivate->demuxer.getVideoStreamIndex();
    if(videoStreamIndex == -1) {
        int ret = 3;
        errorStringVector[ret] = "There is no stream for video, myabe its an audio file?";
        std::cerr << errorStringVector[ret] << std::endl;
        return ret;
    }
    AVStream *videoStream = inputVideoFormatContext->streams[videoStreamIndex];
    int audioStreamIndex = this->pPrivate->demuxer.getAudioStreamIndex();
    if(audioStreamIndex != -1)
        this->pPrivate->info.audioCodecId = inputVideoFormatContext->streams[audioStreamIndex]->codec->codec_id;
    else
        this->pPrivate->info.audioCodecId = AV_CODEC_ID_NONE;

    this->pPrivate->info.duration = inputVideoFormatContext->duration;
    this->pPrivate->info.numberOfStreams = inputVideoFormatContext->nb_streams;
    this->pPrivate->info.frameCounts = videoStream->nb_frames;
    this->pPrivate->info.fps = videoStream->r_frame_rate.num/videoStream->r_frame_rate.den;
    this->pPrivate->info.height = this->pPrivate->demuxer.getHeight();
    this->pPrivate->info.width = this->pPrivate->demuxer.getWidth();
    this->pPrivate->info.filePath.assign(inputVideoFormatContext->filename);
    this->pPrivate->info.videoCodecId = inputVideoFormatContext->video_codec_id;

    uint numberOfChannel = 3; // \WARN number of channle could be 1 for gray scale videos
    unsigned char* decodedFrame = (unsigned char*)malloc(this->pPrivate->demuxer.getWidth()*this->pPrivate->demuxer.getHeight()*numberOfChannel);
    AVPacket avPacket;
    uint frameNumber = 0;
    float *progress = &this->pPrivate->decodedMessage->progress;
    if(_progress != NULL)
        *_progress = progress;
    while(this->pPrivate->demuxer.getNextAvPacketFrame(avPacket)) {
        if(avPacket.stream_index == this->pPrivate->demuxer.getVideoStreamIndex()) { // if it's video frame decode it
            if(this->pPrivate->avMediaDecoder.decodeFrame(avPacket, decodedFrame) == false) {
                std::cerr << "Could not decode frame: " << frameNumber++ << std::endl;
                continue;
            } else {
                std::cout << "frame decoded: " << frameNumber++ <<  std::endl;
            }
            if(frameNumber % 10 == 0)
                std::cout << "progress: " << *progress << std::endl;
        }
        if(extractedInfo == false) {
            if(pPrivate->decodedMessage->messageSection == 4) {
                this->pPrivate->info.messageFilePath = _outputMessageFilePath;
                this->pPrivate->info.messageFilePath += ".";
                this->pPrivate->info.messageFilePath += this->pPrivate->decodedMessage->extension;
                this->pPrivate->info.messageLength = this->pPrivate->decodedMessage->messageLength / 8;
                this->pPrivate->info.messageType = strcmp(this->pPrivate->decodedMessage->extension, "txt") == 0 ? "text" : "file";
                extractedInfo = true;
            }
        }
        if(this->pPrivate->decodedMessage->messageSection == 4 && this->pPrivate->decodedMessage->isBusy == false) {
            this->pPrivate->storeMessage(_outputMessageFilePath);
            break;
        }
    }
    int ret = 0;
    if(this->pPrivate->decodedMessage->messageSection == 4) {
        if(this->pPrivate->decodedMessage->cursor == this->pPrivate->decodedMessage->messageLength)
            std::cout << "Embeded message retrieved and stored in " << _outputMessageFilePath  << "." <<  this->pPrivate->decodedMessage->extension << std::endl;
        else {
            ret = 5;
            errorStringVector[ret] = "The message was uncomplete. messgeLrngth: " + std::to_string(this->pPrivate->decodedMessage->messageLength) + " cursor: " + std::to_string(this->pPrivate->decodedMessage->cursor);
            std::cout << errorStringVector[ret] << std::endl;
        }
    }
    else {
        ret = 6;
        errorStringVector[ret] = "There was no message hidden in " + _videoFilePath;
        std::cout << errorStringVector[ret] << std::endl;
    }
    return ret;
}

void Destegano::getProgress(float &_progress)
{
    _progress = this->pPrivate->decodedMessage->progress;
}

void Destegano::getInfo(stuVideoInfo **_info)
{
    *_info = &this->pPrivate->info;
}



