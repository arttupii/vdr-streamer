#!/bin/sh

INPUT=$1
TARGET_FOLDER=$2
TARGET_FILE=$3
PIDFILE=$4


nice -n 10 cvlc  $INPUT --sub-language fin,fi,suomi  --sout-x264-preset=superfast --audio-language fin,fi,suomi --sout '#transcode{vcodec=h264,vb=2000, acodec=mpga, ab=128, soverlay}:standard{access=file,mux=asf,deinterlace,dst="'$TARGET_FOLDER/$TARGET_FILE'.tmp"}' & 

echo HELLO
PID=$!
echo $PID > $PIDFILE

wait $PID 

mv "$TARGET_FOLDER/$TARGET_FILE.tmp" "$TARGET_FOLDER/$TARGET_FILE.ts"
