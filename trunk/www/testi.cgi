#!/bin/sh

printf "HTTP/1.0 200 OK\r\n"
printf "Content-Type: text/html; charset=UTF-8\r\n"
printf "\r\n"

printf "<html><body>"

read var1


printf '<textarea rows="20" cols="100">'

echo "params --> $var1"

echo "HELLO WORLD"

ls -l -R www/

printf "</textarea>"
printf "</body></html>"

