#include "VideoSteg.h"
#include "Stegano.h"
#include "Destegano.h"
#include <fstream>
#include <iostream>
#include <functional>
#include <stdlib.h>
#include "Private/ErrorStructure.hpp"
//#include "Private/ffmpeg/ffmpeg.h"

std::vector<std::string> errorStringVector = {"Successfull", // 0
                                              "Could not open video file", // 1
                                               "Could not open output file", // 2
                                               "There is no stream for video, myabe its an audio file?", // 3
                                               "The message couldn't be embeded due to its size", // 4
                                               "The message was uncomplete.", // 5
                                               "There was no message hidden", // 6
                                              "Could no open key file", // 7
                                              "The stegano module is not initialized yet!", // 8
                                              "A message already is sending.", // 9,
                                              "0" // 10 - max bit embeded // 10
                                              };

Stegano     stegano;
Destegano   destegano;

int setKeyNumber(const char *_keyFilePath) {
    std::ifstream keyFileDescriptor(_keyFilePath);
    int ret = 0;
    if(keyFileDescriptor.is_open() == false) {
        ret = 7;
        errorStringVector[ret] = "Could no open key file";
        std::cerr << errorStringVector[ret] << std::endl;
        return ret;
    }
    std::string keyContent;
    std::getline(keyFileDescriptor, keyContent, (char)keyFileDescriptor.eof());
    Stegano::key = std::hash<std::string>{}(keyContent);
    return ret;
}

int initEmbedding(const char *inputFilePath, const char *messageFilePathOrMessageText, float **progress, void (*RPCallBack)(float), const char *keyFilePath=NULL, const char *outputFilePath="") {
    int ret;
    if(keyFilePath) {
    ret = setKeyNumber(keyFilePath);
    if(ret != 0)
        return ret;
    }
    ret = stegano.openVideo(inputFilePath, outputFilePath);
    if(ret != 0)
        return ret;
    ret = stegano.embed(messageFilePathOrMessageText);
    if(ret != 0)
        return ret;
    stegano.getProgress(progress);
    if(RPCallBack != NULL) {
        RPCallBack = [](float _progress){stegano.getProgress(_progress);};
    }
    return ret;
}

int Embed(const char *inputFilePath, const char *messageFilePathOrMessageText, const char *outputFilePath, const char *keyFilePath, float **progress, void (*RPCallBack)(float))
{
    int ret = initEmbedding(inputFilePath, messageFilePathOrMessageText, progress, RPCallBack,keyFilePath, outputFilePath);
    if(ret != 0)
        return ret;
    ret = stegano.startEncoding();
    return ret;
}

int Extract(const char *inputFilePath, const char *outputMessageFilePath, const char *keyFilePath, float **progress, void (*RPCallBack)(float))
{
    int ret  = setKeyNumber(keyFilePath);
    if (ret != 0)
        return ret;
    destegano.extractMessage(inputFilePath, outputMessageFilePath, progress);
    if(RPCallBack != NULL) {
        RPCallBack = [](float _progress){destegano.getProgress(_progress);};
    }
    return ret;
}

int FileAndMessageInfo(const char *inputFilePath, const char *messageFilePathOrMessageText, stuVideoInfo **info, float **progress, void (*RPCallBack)(float))
{
    int ret = initEmbedding(inputFilePath, messageFilePathOrMessageText, progress, RPCallBack);
    stegano.getInfo(info);
    return ret;
}

void error2String(int reportNumber, std::string &reportsStr)
{
    reportsStr = errorStringVector[reportNumber];
}

int EmbedNetworkStreaming(const char *inputFilePath, const char *messageFilePathOrMessageText, const char* keyFilePath, float **progress, void (*RPCallBack)(float))
{
    initEmbedding(inputFilePath, messageFilePathOrMessageText, progress, RPCallBack, keyFilePath);
    stegano.startStreaming();
}
