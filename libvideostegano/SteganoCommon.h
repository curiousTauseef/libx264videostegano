#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <vector>

struct stuVideoInfo
{
    float           fps=0;
    uint            width=0;
    uint            height=0;
    uint            duration=0;
    std::string     filePath;
    uint            frameCounts=0;
    int             videoCodecId;
    int             audioCodecId;
    uint            maximumCapacity=0;
    uint            numberOfStreams=0;

    std::string     messageType;
    int             messageLength=0;
    std::string     messageFilePath;
    std::string toString() {
        std::string output = "fps: " + std::to_string(fps) +
                " width: " + std::to_string(width) +
                " height: " + std::to_string(height) +
                " duration: " + std::to_string(duration) +
                " frameCounts: " + std::to_string(frameCounts) +
                " messageLength: " + std::to_string(messageLength);
        return output;
    }
};



#endif
