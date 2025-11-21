#!/bin/sh
set -eu

# Start fcgiwrap on a unix socket, owned by www-data so nginx can access it
spawn-fcgi -u www-data -g www-data \
  -s /run/fcgiwrap.sock \
  -b 1024 \
  -F2 -M 766 /usr/sbin/fcgiwrap

# No daemon mode, keep container in foreground
mkdir -p /var/www/uploads
chown 101:101 /var/www/uploads
exec nginx -g 'daemon off;'
