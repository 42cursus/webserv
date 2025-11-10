#!/bin/sh
# /var/www/cgi-bin/fast_echo.sh

printf "Status: 200 OK\r\n"
printf "Content-Type: application/octet-stream\r\n"

if [ -n "$CONTENT_LENGTH" ]; then
  printf "Content-Length: %s\r\n" "$CONTENT_LENGTH"
fi

printf "\r\n"

# Copy stdin to stdout
if [ -n "$CONTENT_LENGTH" ]; then
  dd bs=1 count="$CONTENT_LENGTH" status=none
else
  cat
fi
