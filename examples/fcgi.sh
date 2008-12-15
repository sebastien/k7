#!/bin/sh
# This simple script runs Lighttpd with 'fcgi.js' as FastCGI script
ROOT=`dirname \`pwd\``
cat fcgi.conf | sed "s|ROOT|$ROOT|g" > /tmp/k7-fcgi.conf
lighttpd -D -f /tmp/k7-fcgi.conf
