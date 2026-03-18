#!/bin/sh

TMPDIR=/dev/shm
[ -d "$TMPDIR" ] || TMPDIR=/tmp
body="$(mktemp "$TMPDIR/cgi-body-XXXXXX")" || exit 1
trap 'rm -f "$body"' EXIT

len="${CONTENT_LENGTH:-0}"
if [ "$len" -gt 0 ] 2>/dev/null; then
	dd bs=1 count="$len" status=none >"$body"
else
	: >"$body"
fi

printf "Status: 200 OK\r\n"
printf "Content-Type: text/plain\r\n"
printf "\r\n"
printf "SCRIPT_NAME=%s\n" "${SCRIPT_NAME:-}"
printf "PATH_INFO=%s\n" "${PATH_INFO:-}"
printf "QUERY_STRING=%s\n" "${QUERY_STRING:-}"
printf "BODY="
cat "$body"
