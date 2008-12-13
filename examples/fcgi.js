var f = new net.http.server.fcgi.FCGI();
while (f.accept() >= 0) {
    f.putstr("Content-type: text/html\r\n\r\nHello from K7");
}
