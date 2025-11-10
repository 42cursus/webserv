#!/bin/sh
TMPDIR=/dev/shm; [ -d "$TMPDIR" ] || TMPDIR=/tmp
body="$(mktemp "$TMPDIR/cgi-body-XXXXXX")" || exit 1
trap 'rm -f "$body" "$out"' EXIT

len="${CONTENT_LENGTH:-}"
if [ -n "$len" ]; then
    blocks=$(( (len + 1048575) / 1048576 ))
    dd bs=1M count="$blocks" iflag=fullblock status=none | head -c "$len" >"$body"
else
    cat >"$body"
fi

out="$(mktemp "$TMPDIR/cgi-out-XXXXXX")" || exit 1
/var/www/cgi-bin/ubuntu_cgi_tester <"$body" >"$out"

#printf "Content-Length: %s\r\n" "$len"

cat "$out"
