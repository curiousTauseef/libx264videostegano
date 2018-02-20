#include "clsX264encoder.h"
#include <string>




using namespace std;

clsX264Encoder::clsX264Encoder()
{
    av_init_packet(&this->avPacket);
    this->avPacket.data = NULL;
    this->avPacket.size = 0;
    this->encoder = NULL;
    this->convertContext = NULL;
    this->initialized = false;
    this->message = new stuMessage();
    this->message->isSendingMessage = 0;
    // baseline and main profile are not supported yet
    this->encodingProfiles = {"baseline", "main","high", "high10", "high422", "high444"};
    //    this->encodingProfiles = {"high"};
}

clsX264Encoder::~clsX264Encoder()
{
    sws_freeContext(convertContext);
    std::cout << "clsX264Encoder::~clsX264Encoder()" << std::endl;
}

bool clsX264Encoder::init(int _width, int _height, uint _fps_num, uint _fps_denum, int _encodingProrfileIndex)
{
    x264_param_default_preset(&parameters, "veryfast", "zerolatency");

    parameters.i_log_level = X264_LOG_NONE;
    parameters.i_threads = 1;
    parameters.i_width = _width;
    parameters.i_height = _height;
    parameters.i_fps_num = _fps_num;
    parameters.i_fps_den = _fps_denum;
    parameters.vui.i_sar_height=1;
    parameters.vui.i_sar_width=1;
    parameters.i_keyint_max = 25;

    parameters.b_intra_refresh = 1;
    parameters.rc.i_rc_method = X264_RC_CRF;
    parameters.rc.i_vbv_buffer_size = 1000000;
    parameters.rc.i_vbv_max_bitrate = 90000;
    parameters.rc.f_rf_constant = 25;
    parameters.rc.f_rf_constant_max = 35;
    this->profileIndex =  _encodingProrfileIndex;
    parameters.i_sps_id = 7;
    // the following two value you should keep 1
    parameters.b_repeat_headers = 1;    // to get header before every I-Frame
    parameters.b_annexb = 1; // put start code in front of nal. we will remove start code later

    //    parameters.i_timebase_num = _fps_denum;
    //    parameters.i_timebase_den = _fps_num;
    x264_param_apply_profile(&parameters, this->encodingProfiles[_encodingProrfileIndex].c_str());
    parameters.message = this->message;

    encoder = x264_encoder_open(&parameters);
    x264_picture_alloc(&picture_in, X264_CSP_I420, parameters.i_width, parameters.i_height);
    picture_in.i_type = X264_TYPE_AUTO;
    picture_in.img.i_csp = X264_CSP_I420;
    // I have initilized my color space converter for BGR24 to YUV420 because my opencv video capture gives BGR24 image. You can initilize according to your input pixelFormat
    convertContext = sws_getContext(parameters.i_width,parameters.i_height, AV_PIX_FMT_RGB24, parameters.i_width,parameters.i_height,AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, NULL, NULL, NULL);
//    if(parameters.b_repeat_headers) {
//        std::cout << "parameters.b_repeat_headers" << std::endl;
//        x264_nal_t* nals ;
//        int i_nals = 0;
//        int frame_size = -1;
//        uint cursor = 0;
//        frame_size = x264_encoder_headers(encoder, &nals, &i_nals);
//        this->avPacket.size = frame_size;
//        this->avPacket.data = (uint8_t*) malloc(frame_size * sizeof(uint8_t));
//        for(int i = 0; i< i_nals; i++) {
//            memcpy(this->avPacket.data + cursor, nals[i].p_payload, nals[i].i_payload);
//            unsigned char videoFrameType = nals[i].p_payload[4];
//            if (videoFrameType == 0x68)
//            {
//                if (ppsFrame != NULL)
//                {
//                    delete ppsFrame; ppsFrameLength = 0;
//                    ppsFrame = NULL;
//                }
//                ppsFrameLength = nals[i].i_payload;
//                ppsFrame = new unsigned char[ppsFrameLength];
//                memcpy(ppsFrame, nals[i].p_payload, ppsFrameLength);
//            }
//            else if (videoFrameType == 0x67)
//            {
//                // sps
//                if (spsFrame != NULL)
//                {
//                    delete spsFrame; spsFrameLength = 0;
//                    spsFrame = NULL;
//                }
//                spsFrameLength = nals[i].i_payload;
//                spsFrame = new unsigned char[spsFrameLength];
//                memcpy(spsFrame, nals[i].p_payload, spsFrameLength);
//            }
//            cursor += nals[i].i_payload;
//            output_queue.push(nals[i]);
//        }
//    }
    this->initialized = true;
    return this->initialized;
}

bool clsX264Encoder::reinit()
{
    return this->init(this->parameters.i_width, this->parameters.i_height, parameters.i_fps_num, parameters.i_fps_den, this->profileIndex);
}


//Encode the rgb frame into a sequence of NALs unit that are stored in a std::vector
AVPacket &clsX264Encoder::encodeFrame(unsigned char *rgb_buffer)
{
    const uint8_t * rgb_buffer_slice[1] = {(const uint8_t *)rgb_buffer};
    int stride = 3 * parameters.i_width; // RGB stride
    //Convert the frame from RGB to YUV420
    int sliceHeight = sws_scale(convertContext, rgb_buffer_slice, &stride, 0, parameters.i_height, picture_in.img.plane, picture_in.img.i_stride);
    if(sliceHeight <= 0) {
        std::cerr << "Could not convert RGB to YUV" << std::endl;
        throw 1;
    }
    x264_nal_t* nals ;
    int i_nals = 0;
    int frame_size = -1;
    uint cursor = 0;
    //    picture_in.i_dts = this->frameNumbe;
    //    picture_in.i_pts = this->frameNumbe++;
    frame_size = x264_encoder_encode(encoder, &nals, &i_nals, &picture_in, &picture_out);
    if(frame_size > 0) {
        this->avPacket.size = frame_size;
        this->avPacket.data = (uint8_t*) malloc(frame_size * sizeof(uint8_t));
        for(int i = 0; i< i_nals; i++) {
            memcpy(this->avPacket.data + cursor, nals[i].p_payload, nals[i].i_payload);
            unsigned char videoFrameType = nals[i].p_payload[4];
            if (videoFrameType == 0x68)
            {
                if (ppsFrame != NULL)
                {
                    delete ppsFrame; ppsFrameLength = 0;
                    ppsFrame = NULL;
                }
                ppsFrameLength = nals[i].i_payload;
                ppsFrame = new unsigned char[ppsFrameLength];
                memcpy(ppsFrame, nals[i].p_payload, ppsFrameLength);
            }
            else if (videoFrameType == 0x67)
            {
                // sps
                if (spsFrame != NULL)
                {
                    delete spsFrame; spsFrameLength = 0;
                    spsFrame = NULL;
                }
                spsFrameLength = nals[i].i_payload;
                spsFrame = new unsigned char[spsFrameLength];
                memcpy(spsFrame, nals[i].p_payload, spsFrameLength);
            }
            cursor += nals[i].i_payload;
            output_queue.push(nals[i]);
        }
    }

    return this->avPacket;
}



bool clsX264Encoder::isNalsAvailableInOutputQueue()
{
    if(output_queue.empty() == true) {
        return false;
    } else {
        return true;
    }
}

x264_nal_t clsX264Encoder::getNalUnit()
{
    x264_nal_t nal;
    nal = output_queue.front();
    output_queue.pop();
    return nal;
}

