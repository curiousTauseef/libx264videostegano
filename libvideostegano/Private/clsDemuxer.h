#ifndef DEMUXER_H
#define DEMUXER_H
#include <string>
#include <iostream>
extern "C" {
#include <libavformat/avformat.h>
}
/**
 * @brief The clsDemuxer class is aimed for openning video
 */
class clsDemuxer
{
public:
    /**
     * @brief clsDemuxer default constructor
     */
    clsDemuxer();
    ~clsDemuxer();
    /**
     * @brief initialze all variables for opening video
     * @param _fileName input video file path
     * @return true if no error occured
     */
    bool init(const char* _fileName);
    bool reinit();
    /**
     * @brief getNextAvPacketFrame get next coded video or audio frame in AVPacket
     * @param _avPacket
     * @return true if no error occured
     */
    bool getNextAvPacketFrame(AVPacket &_avPacket);
    /**
     * @brief getFileName
     * @return returns file path of opened video
     */
    std::string getFileName() const {return this->fileName;}
    /**
     * @brief getAvFormatContext
     * @return
     */
    AVFormatContext *getAvFormatContext() const {return this->pAvFormatContext;}
    void dumpFormatContext() {av_dump_format(this->pAvFormatContext, 0, this->fileName.c_str(), 0);}
    /**
     * @brief getVideoStreamIndex
     * @return
     */
    int getVideoStreamIndex();
    /**
     * @brief getAudioStreamIndex
     * @return
     */

    int getAudioStreamIndex();
    /**
     * @brief getWidth
     * @return
     */
    uint getWidth();
    /**
     * @brief getHeight
     * @return
     */
    uint getHeight();

private:
    AVFormatContext *pAvFormatContext = NULL;
    AVPacket avPacket;
    std::string fileName;
    bool initialized = false;


};



#endif
