var print = system.shell.print
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
