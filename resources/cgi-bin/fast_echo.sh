#!/bin/sh
# /var/www/cgi-bin/fast_echo.sh

len="${CONTENT_LENGTH:-0}"

printf "Status: 200 OK\r\n"
printf "Content-Type: application/octet-stream\r\n"

if [ -n "${CONTENT_LENGTH}" ]; then
  printf "Content-Length: %s\r\n" "$CONTENT_LENGTH"
fi

printf "\r\n"

if [ "${len}" -gt 0 ]; then
  head -c "${len}"
fi
