#!/bin/sh
len="${CONTENT_LENGTH:-0}"

if [ "$len" -gt 0 ]; then
  blocks=$(( (len + 1048575) / 1048576 ))   # ceil(len / 1MiB)
  dd bs=1M count="$blocks" iflag=fullblock status=none | head -c "$len" >/dev/null
fi

# Respond with a small body
printf "Status: 200 OK\r\n"
printf "Content-Type: text/plain\r\n"
printf "Content-Length: 3\r\n"
printf "\r\n"
printf "ok\n"
