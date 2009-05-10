#!/bin/sh
# This simple script runs Lighttpd with 'fcgi.js' as FastCGI script

cd $(dirname $0)
k7dir=$(dirname $(dirname $PWD))
echo "

Once it starts up, load
  http://localhost:8888/applications/fcgi/index.js
in your web browser for enjoyment of fcgi javascripts!

"

sed "s|ROOT|$k7dir|g" < $PWD/fcgi.conf > /tmp/k7-fcgi.conf && \
	lighttpd -D -f "/tmp/k7-fcgi.conf"

