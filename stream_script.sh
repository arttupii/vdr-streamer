#!/bin/bash

INPUT=$1
PIDFILE=$2
QUALITY=$3

printf "HTTP/1.0 200 OK\r\n"
printf "Content-Type: video/x-ms-asf\r\n"
printf "Pragma: features=broadcast\r\n"
printf "\r\n"

#killall vlc

#cvlc  $INPUT --sub-language fin,fi,suomi --audio-language fin,fi,suomi --sout '#transcode{vcodec=h264,vb=600, scale=0.1, acodec=mpga, ab=96, soverlay}:standard{access=file,mux=asf,deinterlace,dst=:-}'  

#cvlc  $INPUT --sub-language fin,fi,suomi --audio-language fin,fi,suomi --sout '#transcode{vcodec=h264,vb=600, scale=0.1, acodec=mpga, ab=96, soverlay}:standard{access=http,mux=asf,deinterlace,dst=:'$PORT'}' & 

#

if [ "$QUALITY" = "high" ] 
then
	cvlc $INPUT  vlc://quit --sout-x264-quiet --sout-x264-preset=superfast -v 0 --sub-language fin,fi,suomi --audio-language fin,fi,suomi --sout '#transcode{vcodec=h264, scale=1,vb=800,acodec=mp3a,ab=128, soverlay, venc=x264{keyint=120,bpyramid,chroma-me,me-range=8,ref=1,ratetol=1.0,8x8dct,mixed-refs,direct=auto,direct-8x8=-1,non-deterministic,scenecut=50}}:standard{access=file,mux=ts,dst=-'} &
else
	cvlc $INPUT  vlc://quit --sout-x264-quiet --sout-x264-preset=superfast -v 0 --sub-language fin,fi,suomi --audio-language fin,fi,suomi --sout '#transcode{vcodec=h264, scale=0.5,vb=800,acodec=mp4a,ab=128, soverlay, venc=x264{keyint=120,bpyramid,chroma-me,me-range=8,ref=1,ratetol=1.0,8x8dct,mixed-refs,direct=auto,direct-8x8=-1,non-deterministic,scenecut=50}}:standard{access=file,mux=ts,dst=-'} &
fi	

#cvlc $INPUT -v 0 --sub-language fin,fi,suomi --audio-language fin,fi,suomi --sout transcode{vcodec=FLV1,acodec=mp3,vb=200,deinterlace,fps=25,samplerate=44100,ab=32}standart{access=http,mux=ffmpeg{mux=flv},dst=-}
#cvlc $INPUT -v 0 --sub-language fin,fi,suomi --audio-language fin,fi,suomi --sout '#transcode{soverlay,ab=128,samplerate=48000,channels=2,acodec=mpga,vcodec=h264,width=480,height=264,fps=25,vb=500,venc=x264{vbv-bufsize=1200,partitions=all,level=12,no-cabac,subme=7,threads=4,ref=2,mixed-refs=1,bframes=0,min-keyint=1,keyint=50,trellis=2,direct=auto,qcomp=0.0,qpmax=51}}:standard{access=file,mux=asf,dst=-' &


PID=$!

echo $PID > $PIDFILE

wait $PID 
rm STREAMER_PID
