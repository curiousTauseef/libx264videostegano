cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0015 NEW)

project(testStegano)
INCLUDE_DIRECTORIES(../libvideostegano/include ../ffmpeg)
LINK_DIRECTORIES(../libvideostegano/lib ../libx264/lib ../ffmpeg/lib)
set(WITH_LIVEMEDIA 1)
add_definitions(-DWITH_LIVEMEDIA=${WITH_LIVEMEDIA})
if(${WITH_LIVEMEDIA})
message("setting liblivemedia library")
set(LIVEMEDIA_LIBRARY ${CMAKE_SOURCE_DIR}/../liblivemedia/liveMedia/libliveMedia.a
    ${CMAKE_SOURCE_DIR}/../liblivemedia/BasicUsageEnvironment/libBasicUsageEnvironment.a
    ${CMAKE_SOURCE_DIR}/../liblivemedia/groupsock/libgroupsock.a
    ${CMAKE_SOURCE_DIR}/../liblivemedia/UsageEnvironment/libUsageEnvironment.a)
endif()
set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-std=c++0x")
add_executable(${PROJECT_NAME} "main.cpp")

target_link_libraries(${PROJECT_NAME} -lavformat -lpthread -lavfilter -lswscale -lswresample -lavdevice -lavcodec -lavutil -lpostproc -lx264 -lvideostegano )
if(${WITH_LIVEMEDIA})
    target_link_libraries(${PROJECT_NAME} ${LIVEMEDIA_LIBRARY})
endif()
