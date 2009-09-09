var print  = system.shell.print
var curl   = net.http.client.curl
var result = curl.fetchURL("http://www.google.com")
print("status:"       + result.status)
print("charset:"      + result.charset)
print("mimeType:"     + result.mimeType)
print("responseText:" + result.responseText)
