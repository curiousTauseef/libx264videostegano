#include "Stegano.h"
#if WITH_LIVEMEDIA
#include "Private/clsLiveServerMediaSubsession.h"
#include <BasicUsageEnvironment.hh>
#endif

#include "Private/clsPrivateStegano.h"

#include "Private/ErrorStructure.hpp"
#include "Private/hlpFunctions.h"

#include <fstream>
#include <unistd.h>

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavutil/imgutils.h>
#include<libavutil/opt.h>
#include <libavformat/avformat.h>
#include "Private/ffmpeg/ffmpeg.h"
}

int Stegano::key = 0;


Stegano::Stegano() :
    pPrivate(new clsPrivateStegano)
{
    this->pPrivate->maximumFrameNumber = -1;
    this->pPrivate->info.maximumCapacity = -1;
    this->pPrivate->message=new stuMessage*[1];
    this->pPrivate->message[0] = new stuMessage();
    av_log_set_level(AV_LOG_QUIET);
    av_register_all();
    avcodec_register_all();
    avformat_network_init();
    avdevice_register_all();
}

// \TODO add a key for message embedding
int Stegano::embed(const std::string &_embedFilePath)
{
    if(this->pPrivate->isInitialized() == false) {
        int ret = 8;
        errorStringVector[ret] = "The stegano module is not initialized yet!";
        std::cerr << errorStringVector[ret] << std::endl;
        return ret;
    }
    if((*this->pPrivate->message)->isSendingMessage) {
        int ret = 9;
        errorStringVector[ret] = "A message already is sending.";
        std::cerr << errorStringVector[ret] << std::endl;
        return ret;
    }
    this->pPrivate->info.messageFilePath = _embedFilePath;
    if(clsPrivateStegano::fileExits(_embedFilePath) == false) {
        this->pPrivate->info.messageType = "text";
        this->pPrivate->info.messageFilePath = "rawMessage.txt";
        std::cerr << "Could not find " << _embedFilePath << ", making a text message with: " << _embedFilePath << " in file: " << this->pPrivate->info.messageFilePath << std::endl;
        FILE *pFile = fopen(this->pPrivate->info.messageFilePath.c_str(), "w");
        fwrite(_embedFilePath.c_str(), sizeof(char), _embedFilePath.size(), pFile);
        fclose(pFile);
    } else {
        this->pPrivate->info.messageType = "file";
    }
    messageInit(this->pPrivate->info.messageFilePath.c_str(), (*this->pPrivate->message));
    this->pPrivate->info.messageLength = (*this->pPrivate->message)->length / 8; // byte
    char* startBitBoundary = clsPrivateStegano::startBit();
    memmove((*this->pPrivate->message)->body, startBitBoundary, START_BITS_COUNT * sizeof(char));
    if((*this->pPrivate->message)->length > this->pPrivate->info.maximumCapacity) {
        std::cerr << "This message can not be embedded in this video. Max capaciyt of this vide is " << this->pPrivate->info.maximumCapacity << " but this message length is " << (*this->pPrivate->message)->length << " bits" << std::endl;
        return 4;
    }
    //    this->pPrivate->message->width = this->pPrivate->demuxer.getWidth();
    //    this->pPrivate->message->height = this->pPrivate->demuxer.getHeight();
    return 0;
}

int Stegano::openVideo(const std::string &_videoFilePath, const std::string &_outputFilePath)
{
    if(this->pPrivate->isInitialized()) {
        this->inputFileName = _videoFilePath;
        if(_outputFilePath.length() != 0)
            this->outputFileName = _outputFilePath;
        return 0;
    }
    this->inputFileName = _videoFilePath;
    if (_outputFilePath.length() == 0) {
        uint indexExtension = _videoFilePath.find_last_of(".");
        if(indexExtension < _videoFilePath.size()) {
            outputFileName.insert(outputFileName.begin(), _videoFilePath.begin(), _videoFilePath.begin() + indexExtension);
            outputFileName += "_embeded";
            outputFileName.insert(outputFileName.end(), _videoFilePath.begin()+indexExtension, _videoFilePath.end());
        } else {
            outputFileName = "notSupported.mp4";
        }
    } else {
        outputFileName = _outputFilePath;
    }


    if(unlink(outputFileName.c_str()) == 0)
        std::cout << "Output file " << outputFileName << " exists. It will be replaced with new one.";
    if(this->pPrivate->demuxer.init(_videoFilePath.c_str()) == false) {
        int ret = 1;
        errorStringVector[ret] = "Could not open " + _videoFilePath;
        std::cerr << errorStringVector[ret] << std::endl;
        this->pPrivate->initialized = false;
        return ret;
    }
    this->pPrivate->avMediaDecoder.init(this->pPrivate->demuxer.getAvFormatContext());
    srand(Stegano::key);
    int encodingProfileIndex = rand() % this->pPrivate->x264Encoder.encodingProfiles.size();
    AVFormatContext *inputVideoFormatContext = this->pPrivate->demuxer.getAvFormatContext();
    int videoStreamIndex = this->pPrivate->demuxer.getVideoStreamIndex();
    uint fps_num = inputVideoFormatContext->streams[videoStreamIndex]->r_frame_rate.num;
    uint fps_den = inputVideoFormatContext->streams[videoStreamIndex]->r_frame_rate.den;
    this->pPrivate->x264Encoder.init(this->pPrivate->demuxer.getWidth(), this->pPrivate->demuxer.getHeight(), fps_num, fps_den, encodingProfileIndex);
    //    if(_videoFilePath.find("rtsp") != 0)
    //        if(this->pPrivate->muxer.init(outputFileName.c_str(), this->pPrivate->demuxer.getAvFormatContext(), true) == false) {
    //            int ret = 2;
    //            errorStringVector[ret] = "Could not open output file: " +  outputFileName ;
    //            std::cerr << errorStringVector[ret] << std::endl;
    //            this->pPrivate->initialized = false;
    //            return ret;
    //        }

    AVPacket temp = this->pPrivate->x264Encoder.getAvpacket();
    //    this->pPrivate->muxer.writeFrame(temp);
    uint numberOfChannel = 3;
    this->pPrivate->decodedFrame = (unsigned char*)malloc(this->pPrivate->demuxer.getWidth()*this->pPrivate->demuxer.getHeight()*numberOfChannel);
    this->pPrivate->message = &this->pPrivate->x264Encoder.message;
    if(videoStreamIndex == -1) {
        int ret = 3;
        errorStringVector[ret] = "There is no stream for video, myabe its an audio file?";
        std::cerr <<  errorStringVector[ret] << std::endl;
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
    if(this->pPrivate->info.frameCounts == 0){
        this->pPrivate->info.frameCounts = this->pPrivate->info.fps * this->pPrivate->info.duration/1000000;
    }
    this->pPrivate->info.height = this->pPrivate->demuxer.getHeight();
    this->pPrivate->info.width = this->pPrivate->demuxer.getWidth();
    this->pPrivate->info.filePath.assign(inputVideoFormatContext->filename);
    this->pPrivate->info.videoCodecId = inputVideoFormatContext->video_codec_id;
    this->pPrivate->initialized = true;

    return 0;
}

void Stegano::getProgress(float &_progress)
{
    _progress = (*this->pPrivate->message)->progress;
}

void Stegano::getProgress(float **_progress)
{
    *_progress = &(*this->pPrivate->message)->progress;
}

void Stegano::getInfo(stuVideoInfo **_info)
{
    int capacity = this->getCapacity();
//    int capacity=10000000;
    std::cout << "Capacity: " << capacity << std::endl;
    this->pPrivate->info.maximumCapacity = capacity;
    *_info = &this->pPrivate->info;
}

int Stegano::getCapacity()
{
    if(this->pPrivate->info.maximumCapacity != -1)
        return this->pPrivate->info.maximumCapacity;
    (*this->pPrivate->message)->gettingCapacity = 1;
    (*this->pPrivate->message)->numberOfEmbededBits = 0;
    AVPacket avPacket;
    uint frameNumber = 0;
    uint part = 1;
    while(this->pPrivate->demuxer.getNextAvPacketFrame(avPacket)) {
        if(avPacket.stream_index == this->pPrivate->demuxer.getVideoStreamIndex()) { // if it's video decode it
            if(this->pPrivate->avMediaDecoder.decodeFrame(avPacket, this->pPrivate->decodedFrame) == false) {
                std::cerr << "Could not decode one frame." << std::endl;
                continue;
            }
            frameNumber++;
        } else {
            continue;
        }
        this->pPrivate->x264Encoder.encodeFrame(this->pPrivate->decodedFrame);
        if(frameNumber >= this->pPrivate->info.frameCounts/part && this->pPrivate->info.frameCounts != 0)
            break;
    }
    //    this->pPrivate->demuxer.reinit();
    //    this->pPrivate->avMediaDecoder.reinit(this->pPrivate->demuxer.getAvFormatContext());
    //    this->pPrivate->x264Encoder.reinit();
    (*this->pPrivate->message)->gettingCapacity = 0;
    int numberOfEmbeddedBits = 0;
    if(this->pPrivate->info.frameCounts != 0)
        numberOfEmbeddedBits = part*(*this->pPrivate->message)->numberOfEmbededBits;
    else
        numberOfEmbeddedBits = (*this->pPrivate->message)->numberOfEmbededBits;;
    (*this->pPrivate->message)->numberOfEmbededBits = 0;
    //    this->pPrivate->muxer.reinit(this->pPrivate->demuxer.getAvFormatContext());
    return numberOfEmbeddedBits-int(0.3*numberOfEmbeddedBits);
}


int Stegano::startEncoding()
{
    //    int argc = 12;
    //    const char* tmpArgv[]={"embedding", "-i", , "-c:a", "copy", "-c:v", "libx264", "-tune", "zerolatency", "-preset", "veryfast", this->outputFileName.c_str()};
    //    char** argv2 = const_cast<char**>(tmpArgv);
    stuAvVideoInfo *videoInfo = new stuAvVideoInfo();
    //    videoInfo.audioCodecId = &this->pPrivate->info.audioCodecId;
    videoInfo->duration = &this->pPrivate->info.duration;
    videoInfo->fps = &this->pPrivate->info.fps;
    videoInfo->frameCounts=&this->pPrivate->info.frameCounts;
    videoInfo->height=&this->pPrivate->info.height;
    videoInfo->numberOfStreams=&this->pPrivate->info.numberOfStreams;
    //    videoInfo.videoCodecId=&this->pPrivate->info.videoCodecId;
    videoInfo->width=&this->pPrivate->info.width;
    (*this->pPrivate->message)->isSendingMessage = true;
    embedVideo(this->inputFileName.c_str(), this->outputFileName.c_str(), this->pPrivate->message, videoInfo);
    //    AVPacket avPacket;
    //    uint frameNumber = 0;
    //    bool firstTime = false;
    //    while(this->pPrivate->demuxer.getNextAvPacketFrame(avPacket)) {
    //        if(avPacket.stream_index == this->pPrivate->demuxer.getVideoStreamIndex()) { // if it's video decode it
    //            if(this->pPrivate->avMediaDecoder.decodeFrame(avPacket, this->pPrivate->decodedFrame) == false) {
    //                std::cerr << "Could not decode one frame." << std::endl;
    //                continue;
    //            }
    //        } else { // otherwise mux it to the container
    //            this->pPrivate->muxer.writeFrame(avPacket);
    //            continue;
    //        }
    //        AVPacket tmpPacket = this->pPrivate->x264Encoder.encodeFrame(this->pPrivate->decodedFrame);
    //        avPacket.data = tmpPacket.data;
    //        avPacket.size = tmpPacket.size;
    //        this->pPrivate->muxer.writeFrame(avPacket);
    //        float *progress;
    //        getProgress(&progress);
    //        if(frameNumber % 10 == 0)
    //            std::cout << "Progress is: " << *progress << std::endl;
    //        if(++frameNumber > this->pPrivate->maximumFrameNumber )
    //            break;
    //        if(this->pPrivate->message->isSendingMessage == false && firstTime == false) {
    //            std::cout << "Embeding was successfully done" << std::endl;
    //            firstTime = true;
    ////            break;
    //        }
    //    }
    int ret = 0;
    if((*this->pPrivate->message)->isSendingMessage == true) {
        ret = 4;
        errorStringVector[ret] = "The message couldn't be embeded due to its size! Size of the file is " + std::to_string((*this->pPrivate->message)->length)
                + " bits but this video could embed " + std::to_string((*this->pPrivate->message)->cursor) + " bits of it.";
        std::cerr << errorStringVector[4] << std::endl;
        ret = 10;
        errorStringVector[ret] = std::to_string((*this->pPrivate->message)->cursor);
    }
    //    this->pPrivate->muxer.close(this->pPrivate->x264Encoder.getPpsFrameLength(), this->pPrivate->x264Encoder.getSpsFrameLength(),
    //                                this->pPrivate->x264Encoder.getPpsFrame(), this->pPrivate->x264Encoder.getSpsFrame());
    return ret;
}

#if WITH_LIVEMEDIA
void Stegano::startStreaming()
{
    this->pPrivate->demuxer.reinit();
    this->pPrivate->avMediaDecoder.reinit(this->pPrivate->demuxer.getAvFormatContext());
    this->pPrivate->x264Encoder.reinit();
    TaskScheduler* taskSchedular = BasicTaskScheduler::createNew();
    BasicUsageEnvironment* usageEnvironment = BasicUsageEnvironment::createNew(*taskSchedular);
    RTSPServer* rtspServer = RTSPServer::createNew(*usageEnvironment, 8554, NULL);
    if(rtspServer == NULL) {
        *usageEnvironment << "Failed to create rtsp server ::" << usageEnvironment->getResultMsg() <<"\n";
        std::exit(1);
    }
    ServerMediaSession* sms = ServerMediaSession::createNew(*usageEnvironment, "live", "live", "Live H264 Stream");
    clsLiveServerMediaSubsession *liveSubSession = clsLiveServerMediaSubsession::createNew(*usageEnvironment, true);
    clsVideoFramedSource::encoder = &this->pPrivate->x264Encoder;
    clsVideoFramedSource::decoder = &this->pPrivate->avMediaDecoder;
    clsVideoFramedSource::demuxer = &this->pPrivate->demuxer;
//    clsVideoFramedSource::muxer   = &this->pPrivate->muxer;
    clsVideoFramedSource::message = this->pPrivate->message;

    (*this->pPrivate->message)->isSendingMessage = false;

    sms->addSubsession(liveSubSession);
    rtspServer->addServerMediaSession(sms);
    char* url = rtspServer->rtspURL(sms);
    *usageEnvironment << "Play the stream using url "<< url << "\n";
    delete[] url;
    taskSchedular->doEventLoop();
}
#endif

