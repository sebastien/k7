#!/bin/sh
# This simple script runs Lighttpd with 'fcgi.js' as FastCGI script

cd $(dirname $0)
K7_DIR=$(dirname $(dirname $PWD))
echo "\
K7 HTTP Server Example
======================

ROOT=$K7_DIR

Visit http://localhost:8888/examples/fcgi/index.js
Benchmark 'ab -c4 -n1000 http://localhost:8888/examples/fcgi/index.js'

"

sed "s|ROOT|$K7_DIR|g" < $PWD/fcgi.conf > /tmp/k7-fcgi.conf && \
	lighttpd -D -f "/tmp/k7-fcgi.conf"

# EOF
