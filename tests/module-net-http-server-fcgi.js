var fcgi    = net.http.server.fcgi
var handler = new fcgi.FCGI()
while( handler.accept() == 0 ) {
	print("FCGI << " + handler.read())
	handler.putstr("Content-type: text/html\r\n\r\nHello from K7!")
}
