#!/bin/sh
set -eu

# Start fcgiwrap on a unix socket, owned by www-data so nginx can access it
spawn-fcgi -s /run/fcgiwrap.sock -M 766 -u www-data -g www-data /usr/sbin/fcgiwrap

# No daemon mode, keep container in foreground
exec nginx -g 'daemon off;'
