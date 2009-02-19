#!/bin/sh
# This simple script runs Lighttpd with 'fcgi.js' as FastCGI script

cd `dirname $0` && \
	sed "s|ROOT|$(dirname $(dirname $PWD))|g" < $PWD/fcgi.conf > /tmp/k7-fcgi.conf && \
	lighttpd -D -f "/tmp/k7-fcgi.conf"
