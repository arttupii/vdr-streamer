#!/bin/sh

printf "HTTP/1.0 200 OK\r\n"
printf "Content-Type: text/html; charset=UTF-8\r\n"
printf "\r\n"

printf "<html><body>"

read var1
echo "params --> $var1"

echo "HELLO WORLD"

printf "<PLAINTEXT>"

ls -l -R www/

printf "</PLAINTEXT>"

printf "</body></html>"

