#ifndef AVMEDIADECODER_H
#define AVMEDIADECODER_H
extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavcodec/decodeMessage.h>
}
class Destegano;
/**
 * @brief The clsAvMediaDecoder class is aimed for decoding all media, including audio and video
 */
class clsAvMediaDecoder
{
public:
    /**
     * @brief clsAvMediaDecoder default constructor
     */
    clsAvMediaDecoder();
    ~clsAvMediaDecoder();
    /**
     * @brief initialize variables for decoding
     * @param _pAvFormatContext input pointer AVFormatContext from opened video in demuxer
     * @param _videoStreamIndex index of video stream in video file
     * @return true if no error occured
     */
    bool init(AVFormatContext *_pAvFormatContext, int _videoStreamIndex = -1);
    /**
     * @brief init initialize variables for decoding
     * @param _width width of video
     * @param _height height of video
     * @param _codecId id of video codec
     * @return true if no error occured
     */
    bool init(uint _width, uint _height, enum AVCodecID _codecId);
    bool reinit(AVFormatContext *_pAvFormatContext);
    /**
     * @brief decodeFrame decode audio or video
     * @param _avPacket input video or audio packer
     * @param _decodedFrameData returned decoded frame
     * @return true if no error occured
     */
    bool decodeFrame(AVPacket _avPacket, unsigned char *_decodedFrameData);
    /**
     * @brief get return some properties of opened or deoced video
     * @param propID id of property
     * @return
     */
    uint get(int propID);
    private:
    /**
     * @brief openCodecContext this function opens codec context struct
     * @param streamIdx
     * @param fmtCtx
     * @param type
     * @return
     */
    int openCodecContext(int *streamIdx, AVFormatContext *fmtCtx, enum AVMediaType type);

private:
    int videoStreamIndex = -1;
    int audioStreamIdx = -1;
    enum AVPixelFormat pixelFormat;
    AVCodecContext *pAvVideoCodecContext = NULL;
    AVCodecContext *pAvAudioCodecContext = NULL;
    AVFrame *yuvFrame = NULL;
    AVFrame *rgbFrame = NULL;
    int rgbSize;
    SwsContext* convertRgbYuvcontext = NULL;
    stuDecodedMessage *decodedMessage;

    friend class Destegano;
    friend class Stegano;

};

#endif
