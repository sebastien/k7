var ev = system.event;
var px = net.posix;

var secret_text = "Hello "+Math.random();

function maywrite (fd,evtype) {
    print("May write!\n");
    var str = ev.buffer_new();
    ev.buffer_add(str,secret_text+"\n");
    ev.buffer_write(str,fd);
    px.close(fd);
}

function mayaccept (fd, evtype) {
    var conn = px.accept(fd);
    print("accepted!\n");
    for (val in conn)
        print(val+": "+conn[val]+"\n");
    var newconn = conn.sock;
    ev.make_socket_nonblocking(newconn);
    ev.event_add(newconn,ev.EV_WRITE,{obj:'dummy'},maywrite);
}

function mayread (fd,evtype) {
    var rdbf = ev.buffer_new();
    ev.buffer_read(rdbf,fd,1024);
    var str = ev.buffer_readln(rdbf);
    print("read: '"+str+"'\n");
    if (str==secret_text)
        print("SUCCESS!\n");
    else
        print("FAILURE: "+secret_text+"!="+str+"\n");
    px.close(fd);
}

var sock;
try {
    sock = px.socket_tcp();
    ev.make_socket_nonblocking(sock);
    px.bind(sock,{addr:"127.0.0.1",port:7001});
    print("bound!\n");
    px.listen(sock);
    ev.event_add(sock,ev.EV_READ,{obj:'dummy'},mayaccept);
    print("yep,added\n");
    var client = px.socket_tcp();
    ev.make_socket_nonblocking(client);
    try{ px.connect(client,{addr:"127.0.0.1",port:7001}); } 
    catch (oip) {print("expected err: "+oip+"\n");}
    print("connected\n");
    ev.event_add(client,ev.EV_READ,{obj:'dummy'},mayread);
    ev.event_loop(10000);
} catch (error) {
    print("error in network code: "+error);
}

px.close(sock);

