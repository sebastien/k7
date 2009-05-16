header("X-Generated-By: k7 fcgi");

print(
	"<html>",
		"<head><title>A k7 CGI script</title>",
		"<body>",
		"<h1>Hello, world.</h1>",
		"<p>This is a k7 fcgi script.</p>",
		"<i>@TODO: Make the output of this test script use the JSON module.</i>",
		"<h2>Request Body</h2>",
		"<pre>", this.getRawRequest(), "</pre>",
		"<h2>get</h2>",
		'<form action="',FCGI_ENV.REQUEST_URI,'" method="get">',
		'<input type="hidden" name="something hidden" value="hidden get value">',
		'<input type="submit" name="something" value="Get something">',
		'</form>',
		"<pre>{\n");

for (var i in server.get) {
	print("\t", i, " : ", server.get[i], "\n");
}

print("}</pre>",
		"<h2>post</h2>",
		'<form action="',FCGI_ENV.REQUEST_URI,'" method="post">',
		'<input type="hidden" name="something hidden" value="hidden+post value">',
		'<input type="submit" name="something" value="Post something">',
		'</form>',
		"<pre>{\n");

for (var i in server.post) {
	print("\t", i, " : ", server.post[i], "\n");
}

print("}</pre>",
		"<h2>FCGI Environment:</h2>",
		"<pre>{\n");
for (var i in FCGI_ENV) {
	print("\t", i, " : ", FCGI_ENV[i], "\n");
}
print(
	"}</pre>",
	"<h2>Server Environment</h2>",
	"<pre>{\n"
);
for (var i in ENV) {
	print("\t", i, " : ", ENV[i], "\n");
}
print("}</pre></body></html>");
