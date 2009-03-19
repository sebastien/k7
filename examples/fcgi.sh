#!/bin/sh
# This simple script runs Lighttpd with 'fcgi.js' as FastCGI script
ROOT=`dirname \`pwd\``
cat fcgi.conf | sed "s|ROOT|$ROOT|g" > /tmp/k7-fcgi.conf
echo "Starting lighttpd on http://localhost:8888"
lighttpd -D -f /tmp/k7-fcgi.conf
# EOF
