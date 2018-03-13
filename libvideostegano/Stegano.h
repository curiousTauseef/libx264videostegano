#ifndef STEGANO_H
#define STEGANO_H

#include "SteganoCommon.h"
class clsPrivateStegano;


/**
 * @brief The Stegano class is aimed for embedding a hidden message to a video file
 */
class Stegano
{
public:
    Stegano();

public:
    /**
     * @brief embed a message
     * @param _embedFilePath Address of message file or message text
     * @return  key error
     */
    int embed(const std::string &_embedFilePath);
    /**
     * @brief openVideo tries to open video
     * @param _videoFilePath address of video file
     * @return key error
     */
    int openVideo(const std::string &_videoFilePath, const std::string &_outputFilePath="");
    /**
     * @brief getProgress returns progress of embedding process
     * @param _progress embedding process
     */
    void getProgress(float &_progress);
    /**
     * @brief getProgress
     * @param _progress
     */
    void getProgress(float **_progress);
    /**
     * @brief getInfo information about message and video file
     * @param _info
     */
    void getInfo(stuVideoInfo **_info);

    int getCapacity();

    void dontEmbed();


public:
    /**
     * @brief startEncoding A blocking function for encoding and embedding
     * @return  key error
     */
    int startEncoding();
    /**
     * @brief startStreaming A blocking function for encoding, embedding and streaming
     */
    void startStreaming();

public:
    /**
     * @brief key key number used for seed
     */
    static int                 key;

private:
    /**
     * @brief pPrivate private object pointer
     */
    clsPrivateStegano *pPrivate;
    std::string outputFileName;
    std::string inputFileName;

};

#endif // STEGANO_H
