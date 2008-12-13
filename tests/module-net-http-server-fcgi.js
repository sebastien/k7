var fcgi    = net.http.server.fcgi
var handler = new fcgi.FCGI()
while( fcgi.accept() == 0 ) {
	print("FCGI << " + fcgi.read())
	fcgi.putstr("Content-type: text/html\r\n\r\nHello from K7!")
}
