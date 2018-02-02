#include <fstream>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include "VideoSteg.h"
#include <cmath>

// parsing arguments using getopt unistd function
bool doNetworkStreaming = false;
std::string inputFileName;
std::string hiddenFilePath = "";
std::string keyFilePath = "defaultKey.k";
std::string outputMessageFilePath = "message";
int key = 0;

int usage(char** argv) {
    std::cerr << "Usge: " << argv[0] << "-i <input file>(Required: Address of Input video or Camera Address)"
                                        "-o <output message file>(Optional: Address of output stored message, Default: message.xxx) "
                                        "-k <keyFileAddress>(optional: Address of key file)"
                                        "-h <Address of stegano file>(Required if you want to embed a file)"
                                        "-n(Optional: Stream video over network )"  << std::endl;
    return 1;
}

int inputParser (int argc, char** argv) {
    if(argc < 3)
        return usage(argv);
    opterr = 0;
    int c;

    while((c = getopt(argc, argv, "ni:h:k:o:")) != -1) {
        switch (c) {
        case 'n':
            doNetworkStreaming = true;
            break;
        case 'i':
            inputFileName.assign(optarg);
            break;
        case 'k':
             keyFilePath.assign(optarg);
            break;
        case 'o':
             outputMessageFilePath.assign(optarg);
            break;
        case 'h':
            hiddenFilePath.assign(optarg);
            break;
        case '?':
            if (optopt == 'i' || optopt == 'h' || optopt == 'k')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr,
                         "Unknown option character `\\x%x'.\n",
                         optopt);
            return usage(argv);
        default:
            break;
        }
    }
    if(inputFileName.empty())
        return usage(argv);
    return 0;
}

int main(int argc, char *argv[])
{
    // parse input arguments
    int returnParser = inputParser(argc, argv);
    if(returnParser != 0)
        return returnParser;
    float *progress;
    std::fstream keyFs(keyFilePath, std::fstream::in);
    if(keyFs.is_open() == false) {
        std::cerr << "Warning: keyfile path: " << keyFilePath << " does not  existed, making a default key file: " << keyFilePath << std::endl;
        keyFs.open(keyFilePath, std::fstream::out);
        keyFs << 0;
    }
    keyFs.close();

    if(hiddenFilePath.size() != 0) {
        stuVideoInfo *info;
        FileAndMessageInfo(inputFileName.c_str(), hiddenFilePath.c_str(), &info, &progress);
        std::cout << info->toString() << std::endl;
        int ret = Embed(inputFileName.c_str(), hiddenFilePath.c_str(), keyFilePath.c_str(), &progress);
        std::string err;
        error2String(ret, err);
        std::cout <<info->filePath << "," << info->width << "," <<
                    info->height << "," << info->frameCounts << "," << std::round(double(info->duration)/1000000) << "," <<  err << std::endl;
        std::ofstream result("results.csv", std::ofstream::app);
        result << info->filePath << "," << info->width << "," <<
                  info->height << "," << info->frameCounts << "," << std::round(double(info->duration)/1000000) << "," <<  err << std::endl;
        result.close();
    } else {
        Extract(inputFileName.c_str(), outputMessageFilePath.c_str(), keyFilePath.c_str(), &progress);
    }


    return 0;
}
