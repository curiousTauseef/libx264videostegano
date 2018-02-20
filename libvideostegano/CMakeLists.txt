cmake_minimum_required(VERSION 2.8)

project(videostegano)
cmake_policy(SET CMP0015 NEW)

set(WITH_LIVEMEDIA 1)
add_definitions(-DWITH_LIVEMEDIA=${WITH_LIVEMEDIA})

INCLUDE_DIRECTORIES(../libx264 ../ffmpeg
../liblivemedia/groupsock/include ../liblivemedia/liveMedia/include/
../liblivemedia/BasicUsageEnvironment/include ../liblivemedia/UsageEnvironment/include
 .)

LINK_DIRECTORIES(../libx264/lib ../ffmpeg/lib ../liblivemedia/liveMedia)
set(FFMPEG_LIBRARY -lpthread -lavfilter -lswscale -lswresample -lavdevice -lavcodec -lavutil -lavformat -lpostproc)
if(${WITH_LIVEMEDIA})
message("setting liblivemedia library")
set(LIVEMEDIA_LIBRARY ../liblivemedia/liveMedia/libliveMedia.a ../liblivemedia/BasicUsageEnvironment/libBasicUsageEnvironment.a ../liblivemedia/groupsock/libgroupsock.a  ../liblivemedia/UsageEnvironment/libUsageEnvironment.a)
set(LIVEMEDIA_STREAMING_SOURCE
    Private/clsVideoFramedSource.cpp
    Private/clsLiveServerMediaSubsession.cpp)
endif()

set(SOURCES
    Private/clsX264encoder.cpp
    Private/clsMuxer.cpp
    Private/clsDemuxer.cpp
    Private/clsAvMediaDecoder.cpp
    Private/clsPrivateStegano.cpp
    Private/clsPrivateDestegano.cpp
    Private/hlpFunctions.cpp
    Private/ErrorStructure.hpp
    Private/ffmpeg/ffmpeg.c
    Private/ffmpeg/cmdutils.c
    Private/ffmpeg/config.h
    Private/ffmpeg/ffmpeg_opt.c
    Private/ffmpeg/ffmpeg_filter.c
    Stegano.cpp
    Destegano.cpp
    VideoSteg.cpp
    SteganoCommon.h)



#SET(CMAKE_CXX_FLAGS "-std=c++11")
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Debug)
if(${WITH_LIVEMEDIA})
    add_library(${PROJECT_NAME} SHARED ${SOURCES} ${LIVEMEDIA_STREAMING_SOURCE})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${FFMPEG_LIBRARY} -lx264)
else()
    add_library(${PROJECT_NAME} SHARED ${SOURCES})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${FFMPEG_LIBRARY} -lx264 )
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "VideoSteg.h;SteganoCommon.h")
INSTALL(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/lib
        PUBLIC_HEADER DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/include
)
