#ifndef MUXER_H
#define MUXER_H
#include <vector>
#include <string>
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}
#define STREAM_PIX_FMT AV_PIX_FMT_YUV420P
#define STREAM_FRAME_RATE 30

/**
 * @brief The clsMuxer class is aimed for muxing and storing video file into a container
 */
class clsMuxer
{
public:
    /**
     * @brief clsMuxer
     */
    clsMuxer();
    ~clsMuxer();
    /**
     * @brief initialize variables
     * @param _fileName
     * @param _width
     * @param _height
     * @param _codecName
     * @return
     */
    bool init(const char* _fileName, unsigned int _width, unsigned int _height, const char *_codecName = "H264");
    bool init(const char* _fileName, AVFormatContext *_pInputAvFormatContext, bool _isH264 = true);
    bool reinit(AVFormatContext *_pInputAvFormatContext);
    /**
     * @brief writeFrame writes every video or audio frame
     * @param _packet
     */
    void writeFrame(AVPacket &_packet);
    /**
     * @brief closes muxing and storing
     */
    void close(int _ppsFrameLength, int _spsFrameLength, unsigned char *_ppsFrame, unsigned char *_spsFrame);
    /**
     * @brief isValidStreamIndex
     * @param _index
     * @return
     */
    bool isValidStreamIndex(int _index);
    /**
     * @brief setPInputAvFormatContext
     * @param _pInputAvFormatContext
     */
    void setPInputAvFormatContext(AVFormatContext *_pInputAvFormatContext) {this->pInputAvFormatContext = _pInputAvFormatContext;}
private:
    /**
     * @brief openVideo
     * @return
     */
    bool openVideo();
    /**
     * @brief addVideoStream
     * @param _inputStream
     * @param _streamId
     * @return
     */
    bool addVideoStream(AVStream *_inputStream, int _streamId);
    /**
     * @brief addVideoStream
     * @return
     */
    bool addVideoStream();
    /**
     * @brief allocPicture
     * @param pix_fmt
     * @param width
     * @param height
     * @param picture
     * @return
     */
    bool allocPicture(enum AVPixelFormat pix_fmt, int width, int height, AVFrame* picture);

public:
    bool initialized() {return this->isInitialized;}

private:
    AVOutputFormat* pAvOutputFormat;
    AVFormatContext* pOutputAvFormatContext;
    AVFormatContext* pInputAvFormatContext;
    AVDictionary* avDict = NULL;
    unsigned int width;
    unsigned int height;
    int64_t frameNumber = 0;
    bool isInitialized = false;
    std::vector<int> streamIndex;
    uint videoStreamIndex;
    uint videoindexNumber;
    std::string outputFilename;


};


#endif

