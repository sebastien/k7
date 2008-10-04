var shttpd = net.http.server.shttpd
var Server = shttpd.Server;
var httpd = new Server(8080);
print ("Starting server on port 8080")
httpd.registerURI(
	"/",
	function (request) {
		request.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
		request.print("<html><body>Hello, v8 world</body></html>");
		request.setFlags(shttpd.END_OF_OUTPUT);
	}
)
while (1) { httpd.processRequests() }
