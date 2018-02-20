#ifndef VIDEOSTEG_H
#define VIDEOSTEG_H

#include "SteganoCommon.h"
/**
 * @brief Embeds a message in video H.264
 * @param inputFilePath Addres of input video
 * @param messageFilePathOrMessageText Address of message or message text
 * @param keyFilePath Address of key file
 * @param progress A pointer that returns amount of embedding progress
 * @return Error key
 */
int Embed(const char *inputFilePath, const char *messageFilePathOrMessageText, const char *outputFilePath, const char* keyFilePath, float **progress, void (*RPCallBack)(float prog) = NULL);
/**
 * @brief Extract messages file from a video
 * @param inputFilePath Address of video for message extracting
 * @param outputMessageFilePath Address of message to be stored
 * @param keyFilePath Address of key file
 * @param progress A pointer for showing embedding progress
 * @return Error key
 */
int Extract(const char *inputFilePath, const char *outputMessageFilePath, const char *keyFilePath, float **progress, void(*RPCallBack)(float prog) = NULL);
/**
 * @brief FileAndMessageInfo Proivides information for video file and embedding message
 * @param inputFilePath Address of video file
 * @param messageFilePathOrMessageText Address of message or message text
 * @param keyFilePath Address of key file
 * @param info A structure for exibiting some information about embedding process and video file
 * @param progress A pointer for showing embedding progress
 * @return Error key
 */
int FileAndMessageInfo(const char * inputFilePath, const char * messageFilePathOrMessageText, stuVideoInfo **info, float **progress, void(*RPCallBack)(float prog) = NULL);
/**
 * @brief error2String convert key errors to human readable strings
 * @param reportNumber key error
 * @param reportsStr human readable error
 */

int EmbedNetworkStreaming(const char *inputFilePath, const char *messageFilePathOrMessageText, const char* keyFilePath, float **progress, void (*RPCallBack)(float prog) = NULL);

void error2String(int reportNumber, std::string &reportsStr);
#endif
