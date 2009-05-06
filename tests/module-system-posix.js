var posix = system.posix

var fd    = posix.fopen("posixtest.txt", "w")
posix.fwrite("Lorem ipsum dolor sit amet",1,"Lorem ipsume dolor sit amet".length,fd)
posix.fclose(fd)

var fd    = posix.fopen("posixtest.txt", "r")
var data  = posix.fread(1,1000,fd)
print ("fopen:" + data)
posix.fclose(fd)

var fd    = posix.popen("cat posixtest.txt", "r")
var data  = posix.fread(1,1024,fd)
print ("popen:" + data)
posix.pclose(fd)

var buf = new system.event.Buffer();

buf.push8(11);
var eleven = buf.pull8();
print("eleven == "+eleven+"\n");
