#!/bin/sh

INPUT=$1
PIDFILE=$2

echo HTTP/1.0 200 OK
echo Content-Type: video/ts
echo 


#cvlc  $INPUT --sub-language fin,fi,suomi --audio-language fin,fi,suomi --sout '#transcode{vcodec=h264,vb=600, scale=0.1, acodec=mpga, ab=96, soverlay}:standard{access=file,mux=asf,deinterlace,dst=:-}'  

#cvlc  $INPUT --sub-language fin,fi,suomi --audio-language fin,fi,suomi --sout '#transcode{vcodec=h264,vb=600, scale=0.1, acodec=mpga, ab=96, soverlay}:standard{access=http,mux=asf,deinterlace,dst=:'$PORT'}' & 

cvlc $INPUT -v 0 --sub-language fin,fi,suomi --audio-language fin,fi,suomi --sout '#transcode{vcodec=h264, scale=0.5,vb=500,acodec=mpga,ab=128, soverlay, venc=x264{keyint=120,bpyramid,chroma-me,me-range=8,ref=1,ratetol=1.0,8x8dct,mixed-refs,direct=auto,direct-8x8=-1,non-deterministic,scenecut=50}}:standard{access=file,mux=ts,dst=-' &


PID=$!
echo $PID > $PIDFILE

wait $PID 
