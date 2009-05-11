var event = system.event;
var net = net.posix;

try{
    var buf = new event.Buffer();
    buf.push8(11);
    var eleven = buf.pull8();
    print("eleven == "+eleven+"\n");
} catch (exc) {
    print("exception: "+exc);
}
/*buf.push8(33);
var posix = system.posix;
var fd = posix.open(".test",posix.O_RDWR|posix.O_CREAT|posix.O_TRUNC);
if (fd==undefined) {
    print("file open error");
    exit(1);
}
buf.write(fd);
posix.close(fd);*/

var secret_text = "Hello "+Math.random();

function maywrite (fd,ev) {
    print("May write!\n");
    var str = new event.Buffer();
    str.push_str(secret_text+"\n");
    str.write(fd);
    net.close(fd);
}

function mayaccept (fd, ev) {
    var conn = net.accept(fd);
    print("accepted!\n");
    for (val in conn)
        print(val+": "+conn[val]+"\n");
    var newconn = conn.sock;
    event.make_socket_nonblocking(newconn);
    event.add(newconn,event.EV_WRITE,{obj:'dummy'},maywrite);
}

function mayread (fd,ev) {
    var rdbf = new event.Buffer();
    rdbf.read(fd,1024);
    var str = rdbf.readln();
    print("read: '"+str+"'\n");
    if (str==secret_text)
        print("SUCCESS!\n");
    else
        print("FAILURE: "+secret_text+"!="+str+"\n");
    net.close(fd);
}

var sock;
try {
    sock = net.socket_tcp();
    event.make_socket_nonblocking(sock);
    //net.connect(sock,{addr:"127.0.0.1",port:80});
    net.bind(sock,{addr:"127.0.0.1",port:7001});
    print("bound!\n");
    net.listen(sock);
    event.add(sock,event.EV_READ,{obj:'dummy'},mayaccept);
    print("yep,added\n");
    var client = net.socket_tcp();
    event.make_socket_nonblocking(client);
    try{ net.connect(client,{addr:"127.0.0.1",port:7001}); } catch (oip) {}
    print("connected\n");
    event.add(client,event.EV_READ,{obj:'dummy'},mayread);
    event.loop(10000);
} catch (error) {
    print("error in network code: "+error);
}

net.close(sock);

/*function mayread (fdes, ev) {
    var r = new ev.Buffer();
    r.read(fdes);
    var eleven = r.pull8();
    print("now, eleven == "+eleven+"\n");
}

var fd = posix.open("test",posix.O_RDONLY|posix.O_NONBLOCK);
ev.add(fd,ev.EV_READ,null,mayread);
ev.loop(100);
posix.close(fd);*/
