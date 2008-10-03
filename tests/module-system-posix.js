print ("system.posix.time=" + system.posix.time)
print ("system.posix.fopen=" + system.posix.fopen)
print ("system.posix.fread=" + system.posix.fread)

var posix = system.posix
var fd    = posix.popen("echo Hello !", "r")
var data  = posix.fread(1,1024,fd)
print ("I just read:" + data)
posix.pclose(fd)


