#!/bin/sh
# /var/www/cgi-bin/fast_discard.sh

dd of=/dev/null bs=64k status=none 2>/dev/null

printf "Status: 200 OK\r\n"
printf "Content-Type: text/plain\r\n"
printf "\r\n"
printf "OK\n"
