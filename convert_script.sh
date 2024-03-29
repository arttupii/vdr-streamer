#!/bin/sh

INPUT=$1
TARGET_FOLDER=$2
TARGET_FILE=$3
PIDFILE=$4




#nice -n 10 cvlc  $INPUT --sub-language fin,fi,suomi  --sout-x264-preset=superfast --audio-language fin,fi,suomi --sout '#transcode{vcodec=h264,vb=2000, acodec=mpga, ab=128, soverlay}:standard{access=file,mux=asf,deinterlace,dst="'$TARGET_FOLDER/$TARGET_FILE'.tmp"}' & 
#echo nice -n 10 cvlc `ls -L $INPUT/*.ts` --sout-x264-quiet --sout-x264-preset=superfast -v 0 --sub-language fin,fi,suomi --audio-language fin,fi,suomi --sout '#transcode{vcodec=h264, scale=1,vb=1800,acodec=mpga,ab=128, soverlay, venc=x264{keyint=120,bpyramid,chroma-me,me-range=8,ref=1,ratetol=1.0,8x8dct,mixed-refs,direct=auto,direct-8x8=-1,non-deterministic,scenecut=50}}:standard{access=file,mux=ts,dst='$TARGET_FOLDER/$TARGET_FILE'.tmp' 


nice -n 10 cvlc -vvv `ls -L $INPUT/*.ts` vlc://quit  --sout-keep --sout-x264-quiet -v 0 --sub-language fin,fi,suomi --audio-language fin,fi,suomi --sout '#gather:transcode{vcodec=mp4v, scale=1,vb=1500,acodec=mp4a,ab=128, soverlay,deinterlace {keyint=120,bpyramid,chroma-me,me-range=8,ref=1,ratetol=1.0,8x8dct,mixed-refs,direct=auto,direct-8x8=-1,non-deterministic,scenecut=50}}:standard{access=file,mux=mp4,dst='"$TARGET_FOLDER"/"$TARGET_FILE.tmp"'}' &

#nice -n 10 cvlc -vvv `ls -L $INPUT/*.ts` vlc://quit  --sout-keep --sout-x264-quiet --sout-x264-preset=superfast -v 0 --sub-language fin,fi,suomi --audio-language fin,fi,suomi --sout '#gather:transcode{vcodec=h264, scale=1,vb=1500,acodec=mpga,ab=128, soverlay,deinterlace, venc=x264{keyint=120,bpyramid,chroma-me,me-range=8,ref=1,ratetol=1.0,8x8dct,mixed-refs,direct=auto,direct-8x8=-1,non-deterministic,scenecut=50}}:standard{access=file,mux=mp4,dst='"$TARGET_FOLDER"/"$TARGET_FILE.tmp"'}' &

#echo HELLO
PID=$!
echo $PID > $PIDFILE

wait $PID

mv "$TARGET_FOLDER/$TARGET_FILE.tmp" "$TARGET_FOLDER/$TARGET_FILE.mp4"
