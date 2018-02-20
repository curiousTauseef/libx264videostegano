#!/bin/bash
echo building and installing libx264
echo ------------------------------
sleep 3
cd libx264
./configure --enable-shared --prefix=. --disable-asm --disable-thread
make -j9
make install
echo building and installing ffmpeg
echo ------------------------------
sleep 3
cd ../ffmpeg
./configure --prefix=. --enable-shared --extra-cflags="-gstabs+ -I../libx264" --extra-ldflags=-L../libx264/lib --enable-ffplay --enable-libx264 --enable-gpl --enable-pic  --disable-yasm --disable-pthreads
make -j9
make install
echo building and installing liblivemedia
echo ------------------------------
sleep 3
cd ../liblivemedia
./genMakefiles linux
make -j9
cd ../libvideostegano/
echo building and installing libvideostegano
echo ------------------------------
sleep 3
cmake .
make
make install
echo building testSteganoProject
echo ------------------------------
sleep 3
cd ../testStegano/
mkdir build 2>/dev/null
cd build
cmake ..
make
cd ../..
echo ------------------------------
echo
echo
echo for running project dont forget to add the following line to the LD_LIBRARY_PATH variable
echo export LD_LIBRARY_PATH=${PWD}/libx264/lib:${PWD}/ffmpeg/lib:${PWD}/libvideostegano/lib
