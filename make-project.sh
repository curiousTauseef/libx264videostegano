!#/bin/bash
echo building and installing libx264
cd libx264
./configure --enable-shared --prefix=. --disable-asm --disable-thread
make
make install
echo building and installing ffmpeg
cd ../ffmpeg
./configure --prefix=. --enable-shared --extra-cflags="-gstabs+ -I../libx264" --extra-ldflags=-L../libx264/lib --enable-ffplay --enable-libx264 --enable-gpl --enable-pic  --disable-yasm --disable-pthreads
make -j3
make install
cd ../libvideostegano/
echo building and installing libvideostegano
cmake .
make
make install
echo building testSteganoProject
cd ../testStegano/
cmake .
make
cd ..
echo for running project dont forget to add the following line to the LD_LIBRARY_PATH variable
echo export LD_LIBRARY_PATH=${PWD}/libx264/lib:${PWD}/ffmpeg/lib:${PWD}/libvideostegano/lib
