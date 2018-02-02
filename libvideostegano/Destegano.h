#ifndef DESTEGANO_H
#define DESTEGANO_H

#include <string>
#include "SteganoCommon.h"

class clsPrivateDestegano;
/**
 * @brief The Destegano class is aimed for extracting message from video
 */
class Destegano
{
public:
    /**
     * @brief Destegano default constructor
     */
    Destegano();
    /**
     * @brief extractMessage extracts message
     * @param _videoFilePath Address of video file
     * @param _outputMessageFilePath Address of message to be stored
     * @param _progress amount of progress in embedding process
     * @return
     */
    int extractMessage(const std::string &_videoFilePath, const char *_outputMessageFilePath = "decoded", float **_progress = NULL);
    /**
     * @brief getProgress returns progress amount of embedding
     * @param _progress
     */
    void getProgress(float &_progress);
    /**
     * @brief getInfo returns information about video and message
     * @param _info
     */
    void getInfo(stuVideoInfo **_info);

private:
    clsPrivateDestegano *pPrivate;

};

#endif // DESTEGANO_H
