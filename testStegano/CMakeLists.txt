cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0015 NEW)

project(testStegano)
INCLUDE_DIRECTORIES(../libvideostegano/include ../ffmpeg)
LINK_DIRECTORIES(../libvideostegano/lib ../libx264/lib ../ffmpeg/lib)
set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-std=c++0x")
add_executable(${PROJECT_NAME} "main.cpp")
target_link_libraries(${PROJECT_NAME} -lavformat -lpthread -lavfilter -lswscale -lswresample -lavdevice -lavcodec -lavutil -lpostproc -lx264 -lvideostegano )

