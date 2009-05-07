var buf = new system.event.Buffer();

buf.push8(11);
var eleven = buf.pull8();
print("eleven == "+eleven+"\n");
buf.push8(33);

var posix = system.posix;

var fd = posix.open("test",posix.O_RDWR|posix.O_CREAT|posix.O_TRUNC);
if (fd==undefined) {
    print("file open error");
    exit(1);
}
buf.write(fd);
posix.close(fd);

try {
    var net = net.posix;
    var sock = net.socket_tcp();
    var httpsock = net.socket_tcp();
    net.connect(httpsock,{addr:"127.0.0.1",port:80});
    net.bind(sock,{addr:"127.0.0.1",port:7001});
    print("bound!\n");
    net.listen(sock);
    var conn = net.accept(sock);
    print("accepted!\n");
    for (val in conn)
        print(val+": "+conn[val]+"\n");
} catch (error) {
    print("error in network code: "+error);
}
