#!/bin/bash
videoFileName=$1
extension=${videoFileName#*.}
resultDir=${videoFileName}_result
mkdir -p $resultDir 2>/dev/null
videoOutputName=${resultDir}/result.${extension}
./testStegano  -i $videoFileName -v $videoOutputName
maximumCapacity=`cat currentMaximumCapacity`
maximumCapacity=`echo $maximumCapacity/8-94|bc`
echo embedding a random file with $maximumCapacity bytes
randomFileName='myFile.rnd'
messageFileName='message.rnd'
#head -c ${maximumCapacity} </dev/urandom > $randomFileName
head -c ${maximumCapacity} </dev/urandom > $randomFileName
rm $videoOutputName
./testStegano -i $videoFileName -v $videoOutputName -h $randomFileName > /dev/null
echo retrieving the random embedded message
./testStegano -i $videoOutputName  > /dev/null
mv $randomFileName $resultDir
mv $messageFileName $resultDir

