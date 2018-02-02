#include <iostream>
#include <algorithm>
extern "C" {
#include <libavutil/error.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/time.h>
#include <libavutil/avstring.h>
#include <libavformat/avformat.h>
}
#include "clsMuxer.h"
clsMuxer::clsMuxer(){
}

clsMuxer::~clsMuxer()
{

}
// \NOTE this function has not been validated
bool clsMuxer::init(const char* _fileName, unsigned int _width, unsigned int _height, const char *_codecName)
{
    this->width = _width;
    this->height = _height;
    // Initialize libavcodec, and register all codecs and formats
    av_register_all();
    avformat_alloc_output_context2(&this->pOutputAvFormatContext, NULL, _codecName, _fileName);
    if (this->pOutputAvFormatContext == false) {
        printf("Could not deduce output format from file extension");
        return false;
    }
    this->pAvOutputFormat = this->pOutputAvFormatContext->oformat;
    // add the video stream using default format codecs and initialize the codec


    this->isInitialized = addVideoStream();
    //    returnValue &= openVideo();
    av_dump_format(this->pOutputAvFormatContext, 0, _fileName, 1);
    // open the output file
    if(avio_open(&this->pOutputAvFormatContext->pb, _fileName, AVIO_FLAG_WRITE) < 0) {
        std::cerr << "Could not open " << _fileName << std::endl;
        return false;
    }
    // Write the stream header, if any!
    if(avformat_write_header(this->pOutputAvFormatContext, &this->avDict) < 0) {
        std::cerr << "Error occured when openning output file" << std::endl;
        return false;
    }
    this->pInputAvFormatContext = this->pOutputAvFormatContext;
    return this->isInitialized;
}

bool clsMuxer::init(const char* _fileName, AVFormatContext *_pInputAvFormatContext, bool _isH264)
{

    this->pInputAvFormatContext = _pInputAvFormatContext;
    //    av_log_set_level(AV_LOG_DEBUG);
    this->outputFilename = _fileName;
    AVIOContext *output_io_context = NULL;
    avio_open(&output_io_context, _fileName, AVIO_FLAG_WRITE);
    /** Create a new format context for the output container format. */
    this->pOutputAvFormatContext = avformat_alloc_context();
    /** Associate the output file (pointer) with the container format context. */
    this->pOutputAvFormatContext->pb = output_io_context;
    /** Guess the desired container format based on the file extension. */
    this->pOutputAvFormatContext->oformat = av_guess_format(NULL, _fileName, NULL);
    av_strlcpy((this->pOutputAvFormatContext)->filename, _fileName,
               sizeof((this->pOutputAvFormatContext)->filename));

    if (this->pOutputAvFormatContext == false) {
        printf("Could not deduce output format from file extension");
        return false;
    }
    this->pAvOutputFormat = this->pOutputAvFormatContext->oformat;
    // create all streams with their codecs in pAvFormatContext
    for (int cnt = 0; cnt < this->pInputAvFormatContext->nb_streams; cnt++) {
        AVStream *inputStream = this->pInputAvFormatContext->streams[cnt];
        if(inputStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            this->width = inputStream->codecpar->width;
            this->height = inputStream->codecpar->height;
            this->videoStreamIndex = cnt;
            if(_isH264) {
                if(addVideoStream(inputStream, cnt) == false)
                    return false;
                this->streamIndex.push_back(cnt);
                continue;
            }
        } else if (inputStream->codecpar->codec_type != AVMEDIA_TYPE_AUDIO && inputStream->codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE)
            continue;
        streamIndex.push_back(cnt);
        AVCodecContext *inputCodec = inputStream->codec;
        AVStream *outputStream = avformat_new_stream(this->pOutputAvFormatContext, inputCodec->codec);
        if(outputStream == NULL) {
            std::cerr << "Failed to allocate output stream." << std::endl;
            return false;
        }
        int returnValue = avcodec_copy_context(outputStream->codec, inputCodec);
        if(returnValue < 0) {
            std::cerr << "Failed to copy codec context from input to output." << std::endl;
            return false;
        }
        outputStream->codec->codec_tag = 0;
        if(this->pOutputAvFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
            outputStream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    av_dump_format(this->pOutputAvFormatContext, 0, _fileName, 1);
    // Write the stream header, if any!
    if(avformat_write_header(this->pOutputAvFormatContext, NULL) < 0) {
        std::cerr << "Error occured when openning output file" << std::endl;
        return false;
    }
    this->isInitialized = true;
    return this->isInitialized;
}

bool clsMuxer::reinit(AVFormatContext *_pInputAvFormatContext)
{
    return this->init(this->outputFilename.c_str(), _pInputAvFormatContext,  true);
}


void clsMuxer::writeFrame(AVPacket &_packet)
{
    if(isInitialized == false)
        return;
    if(this->isValidStreamIndex(_packet.stream_index) == false)
        return;
    AVStream *inputStream = this->pInputAvFormatContext->streams[_packet.stream_index];
    AVStream *outputStream = this->pOutputAvFormatContext->streams[_packet.stream_index];
    // copy packet
    _packet.pts = av_rescale_q_rnd(_packet.pts, inputStream->time_base, outputStream->time_base, AVRounding(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
    _packet.dts = av_rescale_q_rnd(_packet.dts, inputStream->time_base, outputStream->time_base, AVRounding(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
    _packet.duration = av_rescale_q(_packet.duration, inputStream->time_base, outputStream->time_base);
    _packet.pos = -1;
    int returnValue = av_interleaved_write_frame(this->pOutputAvFormatContext, &_packet);
    if(returnValue < 0)
        std::cerr << "Error in muxing a packet";

}

void clsMuxer::close(int _ppsFrameLength, int _spsFrameLength, unsigned char *_ppsFrame, unsigned char *_spsFrame)
{
    if(this->isInitialized == false)
        return;
    // Extradata contains PPS & SPS for AVCC format
    if(_ppsFrameLength > 0 && _spsFrameLength > 0) {
    int extradata_len = 8 + _spsFrameLength - 4 + 1 + 2 + _ppsFrameLength - 4;
    this->pOutputAvFormatContext->streams[this->videoStreamIndex]->codec->extradata = (uint8_t*)av_mallocz(extradata_len);
    this->pOutputAvFormatContext->streams[this->videoStreamIndex]->codec->extradata_size = extradata_len;
    this->pOutputAvFormatContext->streams[this->videoStreamIndex]->codec->extradata[0] = 0x01;
    this->pOutputAvFormatContext->streams[this->videoStreamIndex]->codec->extradata[1] = _spsFrame[4+1];
    this->pOutputAvFormatContext->streams[this->videoStreamIndex]->codec->extradata[2] = _spsFrame[4+2];
    this->pOutputAvFormatContext->streams[this->videoStreamIndex]->codec->extradata[3] = _spsFrame[4+3];
    this->pOutputAvFormatContext->streams[this->videoStreamIndex]->codec->extradata[4] = 0xFC | 3;
    this->pOutputAvFormatContext->streams[this->videoStreamIndex]->codec->extradata[5] = 0xE0 | 1;
    int tmp = _spsFrameLength - 4;
    this->pOutputAvFormatContext->streams[this->videoStreamIndex]->codec->extradata[6] = (tmp >> 8) & 0x00ff;
    this->pOutputAvFormatContext->streams[this->videoStreamIndex]->codec->extradata[7] = tmp & 0x00ff;
    int i = 0;
    for (i=0;i<tmp;i++)
        this->pOutputAvFormatContext->streams[this->videoStreamIndex]->codec->extradata[8+i] = _spsFrame[4+i];
    this->pOutputAvFormatContext->streams[this->videoStreamIndex]->codec->extradata[8+tmp] = 0x01;
    int tmp2 = _ppsFrameLength-4;
    this->pOutputAvFormatContext->streams[this->videoStreamIndex]->codec->extradata[8+tmp+1] = (tmp2 >> 8) & 0x00ff;
    this->pOutputAvFormatContext->streams[this->videoStreamIndex]->codec->extradata[8+tmp+2] = tmp2 & 0x00ff;
    for (i=0;i<tmp2;i++)
        this->pOutputAvFormatContext->streams[this->videoStreamIndex]->codec->extradata[8+tmp+3+i] = _ppsFrame[4+i];
    }
    av_write_trailer(this->pOutputAvFormatContext);
    avcodec_close(this->pOutputAvFormatContext->streams[this->videoStreamIndex]->codec);
    // here we should free avframes
    if ((this->pAvOutputFormat->flags & AVFMT_NOFILE) == false)
        /* Close the output file. */
        avio_closep(&this->pOutputAvFormatContext->pb);
    avformat_free_context(this->pOutputAvFormatContext);
}

bool clsMuxer::isValidStreamIndex(int _index)
{
    if(std::find(this->streamIndex.begin(), this->streamIndex.end(), _index) == this->streamIndex.end())
        return false;
    else
        return true;
}


bool clsMuxer::addVideoStream(AVStream *_inputStream, int _streamId)
{
    AVStream *videoStream               = NULL;
    AVCodec *videoCodec          = NULL;
    videoCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    videoStream = avformat_new_stream(this->pOutputAvFormatContext, videoCodec);
    AVCodecContext* codecContext;
    codecContext = videoStream->codec;

    //    AVCodec *videoCodec = avcodec_find_encoder(AV_CODEC_ID_H264/*this->pAvOutputFormat->video_codec*/);
    if(videoCodec == false) {
        std::cerr << "Could not find encoder for " << this->pAvOutputFormat->video_codec << std::endl;
        return false;
    }
    //    AVStream *videoStream = avformat_new_stream(this->pOutputAvFormatContext, videoCodec);
    if(videoStream == false) {
        std::cerr << "Could not allocate stream" << std::endl;
        return false;
    }

    videoStream->id = _streamId;
    //    codecContext = avcodec_alloc_context3(videoCodec);
    if(codecContext == NULL) {
        std::cerr << "Could not allocate an encoding context" << std::endl;
        return false;
    }

//    codecContext->b_frame_strategy = _inputStream->codec->b_frame_strategy;
//    codecContext->channels = _inputStream->codec->channels;
//    codecContext->channel_layout = _inputStream->codec->channel_layout;
//    codecContext->frame_bits = _inputStream->codec->frame_bits;
//    codecContext->sample_rate = _inputStream->codec->sample_rate;
//    codecContext->ticks_per_frame = _inputStream->codec->ticks_per_frame;
//    codecContext->timecode_frame_start = _inputStream->codec->timecode_frame_start;
//    codecContext->extradata = _inputStream->codec->extradata;
//    codecContext->extradata_size = _inputStream->codec->extradata_size;
//    videoStream->attached_pic = _inputStream->attached_pic;
    videoStream->display_aspect_ratio = _inputStream->display_aspect_ratio;
    videoStream->sample_aspect_ratio = _inputStream->sample_aspect_ratio;
    videoStream->disposition = _inputStream->disposition;
//    codecContext->coded_height = _inputStream->codec->coded_height;
//    codecContext->coded_width = _inputStream->codec->coded_width;
//    codecContext->coded_side_data = _inputStream->codec->coded_side_data;
//    codecContext->coded_frame= _inputStream->codec->coded_frame;
    if (av_q2d(_inputStream->codec->time_base) * _inputStream->codec->ticks_per_frame > av_q2d(_inputStream->time_base) &&
            av_q2d(_inputStream->time_base) < 1.0 / 1000) {
        codecContext->time_base = _inputStream->codec->time_base;
        codecContext->time_base.num *= _inputStream->codec->ticks_per_frame;
    } else {
        codecContext->time_base = _inputStream->time_base;
    }

//    codecContext->time_base = _inputStream->codec->time_base;
//    videoStream->time_base = _inputStream->codec->time_base;
//    std::cout << "codecContext->time_base: " << codecContext->time_base.num << "/" << codecContext->time_base.den <<
//                 " _inputStream->codec->time_base: " << _inputStream->codec->time_base.num << "/" << _inputStream->codec->time_base.den << std::endl;
    codecContext->codec_id = AV_CODEC_ID_H264;// this->pAvOutputFormat->video_codec;
    codecContext->bit_rate = _inputStream->codec->bit_rate;

    /* Resolution must be a multiple of two. */
    codecContext->width    = _inputStream->codec->width;
    codecContext->height   = _inputStream->codec->height;
    /* timebase: This is the fundamental unit of time (in seconds) in terms
     * of which frame timestamps are represented. For fixed-fps content,
     * timebase should be 1/framerate and timestamp increments should be
     * identical to 1. */
    videoStream->time_base = _inputStream->codec->time_base;
    videoStream->sample_aspect_ratio = _inputStream->sample_aspect_ratio;
    codecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    std::cout << "\n _inputStream->codec->codec_name: " << _inputStream->codec->codec_name << std::endl;
    codecContext->bit_rate_tolerance = _inputStream->codec->bit_rate_tolerance;
    std::cout << "_inputStream->codec->codec_tag: " << _inputStream->codec->codec_tag << " codecContext->codec_tag: " << codecContext->codec_tag << std::endl;
    codecContext->codec_tag = av_codec_get_tag(this->pAvOutputFormat->codec_tag, videoStream->codec->codec_id);//_inputStream->codec->codec_tag;
    codecContext->time_base       = videoStream->time_base;
    videoStream->avg_frame_rate = _inputStream->codec->framerate;
//    videoStream->codec_info_duration
//    videoStream->r_frame_rate = _inputStream->codec->framerate;;
    codecContext->framerate = _inputStream->codec->framerate;


    codecContext->gop_size      = _inputStream->codec->gop_size; /* emit one intra frame every twelve frames at most */
    codecContext->pix_fmt       = _inputStream->codec->pix_fmt;
    if (codecContext->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
        /* just for testing, we also add B-frames */
        codecContext->max_b_frames = _inputStream->codec->max_b_frames;
    }
    if (codecContext->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
        /* Needed to avoid using macroblocks in which some coeffs overflow.
         * This does not happen with normal video, it just happens here as
         * the motion of the chroma plane does not match the luma plane. */
        codecContext->mb_decision = _inputStream->codec->mb_decision;
    }
    if(this->pOutputAvFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
        codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    if (!videoStream->codec->codec_tag) {
        unsigned int codec_tag;
        if (!this->pAvOutputFormat->codec_tag ||
             av_codec_get_id (this->pAvOutputFormat->codec_tag, _inputStream->codec->codec_tag) == videoStream->codec->codec_id ||
             !av_codec_get_tag2(this->pAvOutputFormat->codec_tag, _inputStream->codec->codec_id, &codec_tag))
            videoStream->codec->codec_tag = _inputStream->codec->codec_tag;
    }

    videoStream->codec->bit_rate       = _inputStream->codec->bit_rate;
    videoStream->codec->rc_max_rate    = _inputStream->codec->rc_max_rate;
    videoStream->codec->rc_buffer_size = _inputStream->codec->rc_buffer_size;
    videoStream->codec->field_order    = _inputStream->codec->field_order;
    videoStream->metadata              = _inputStream->metadata;

    avcodec_parameters_from_context(videoStream->codecpar, codecContext);

    return true;
}

bool clsMuxer::addVideoStream()
{
    AVCodecContext* codecContext;
    AVCodec *videoCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if(videoCodec == false) {
        std::cerr << "Could not find encoder for " << this->pAvOutputFormat->video_codec << std::endl;
        return false;
    }
    AVStream *videoStream = avformat_new_stream(this->pOutputAvFormatContext, videoCodec);
    if(videoStream == false) {
        std::cerr << "Could not allocate stream" << std::endl;
        return false;
    }

    videoStream->id = this->pOutputAvFormatContext->nb_streams - 1;
    videoStream->duration = 10;
    codecContext = avcodec_alloc_context3(videoCodec);
    if(codecContext == NULL) {
        std::cerr << "Could not allocate an encoding context" << std::endl;
        return false;
    }
    codecContext->codec_id = AV_CODEC_ID_H264;// this->pAvOutputFormat->video_codec;

    codecContext->bit_rate = 301131;
    //    codecContext->ticks_per_frame = 1;
    /* Resolution must be a multiple of two. */
    codecContext->width    = this->width;
    codecContext->height   = this->height;
    /* timebase: This is the fundamental unit of time (in seconds) in terms
     * of which frame timestamps are represented. For fixed-fps content,
     * timebase should be 1/framerate and timestamp increments should be
     * identical to 1. */
    //    videoStream->time_base = (AVRational){ 1, STREAM_FRAME_RATE };
    codecContext->time_base       = (AVRational){ 1, STREAM_FRAME_RATE };
    codecContext->framerate = (AVRational){STREAM_FRAME_RATE, 1};
    videoStream->avg_frame_rate = (AVRational){STREAM_FRAME_RATE, 1};

    codecContext->gop_size      = 12; /* emit one intra frame every twelve frames at most */
    codecContext->pix_fmt       = AV_PIX_FMT_YUV420P;

    if(this->pOutputAvFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
        codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    videoStream->codec = codecContext;


    /* copy the stream parameters to the muxer */
    int ret = avcodec_parameters_from_context(videoStream->codecpar, this->pOutputAvFormatContext->streams[0]->codec);
    if (ret < 0) {
        fprintf(stderr, "Could not copy the stream parameters\n");
        return false;
    }

    return true;
}

