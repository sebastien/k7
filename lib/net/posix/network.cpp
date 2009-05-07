// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Victor Grishchenko      <victor.grishchenko@gmail.com>
// ----------------------------------------------------------------------------
// Creation date     : 07-May-2009
// ----------------------------------------------------------------------------

#include "macros.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>


// TODO: Add proper error handling
// #define MODULE "system.posix"
using namespace v8;

int obj2addr (Handle<Object> obj, struct sockaddr_in* addr) {
    HandleScope scope;
    String::AsciiValue a ( obj->Get(String::New("addr")) );
    Handle<Value> p = obj->Get(String::New("port"));
    memset(addr,0,sizeof(struct sockaddr_in));
    if (1!=inet_pton(AF_INET,*a,&(addr->sin_addr.s_addr)))
        return -1;
    if (!p->IsInt32())
        return -2;
    addr->sin_port = htons((uint16_t)p->Uint32Value());
    addr->sin_family = AF_INET;
    return 0;
}

Handle<Object> addr2obj (struct sockaddr_in* addr) {
    HandleScope scope;
    Handle<Object> obj(Object::New());
    obj->Set(String::New("port"),Integer::New(ntohs(addr->sin_port)));
    char str[32];
    inet_ntop(AF_INET,&(addr->sin_addr),str,sizeof(sockaddr_in));
    obj->Set(String::New("addr"),String::New(str));
    return scope.Close(obj);
}

FUNCTION(posix_socket_tcp)
    ARG_COUNT(0);
    int sock = socket(AF_INET,SOCK_STREAM,0);
    return Integer::New(sock);
END

FUNCTION(posix_bind)
    ARG_COUNT(2);
    ARG_int(sock,0);
    ARG_obj(ip,1);
    struct sockaddr_in addr;
    if (0!=obj2addr(ip,&addr))
        THROW("invalid IP address");
    if (0!=bind(sock,(struct sockaddr*)&addr,sizeof addr)) 
        THROW(strerror(errno));
    return Undefined();
END

FUNCTION(posix_listen)
    ARG_COUNT(1);
    ARG_int(sock,0);
    if (listen(sock,8)==-1) 
        THROW(strerror(errno));
    return Undefined();
END

FUNCTION(posix_accept)
    ARG_COUNT(1);
    ARG_int(sock,0);
    struct sockaddr_in addr;
    socklen_t len;
    int newsock = accept(sock,(struct sockaddr*)&addr,&len);
    if (-1==newsock)
        THROW(strerror(errno));
    Handle<Object> addrobj = addr2obj(&addr);
    addrobj->Set(String::New("sock"),Integer::New(newsock));
    return addrobj;
END

FUNCTION(posix_connect)
    ARG_COUNT(2);
    ARG_int(sock,0);
    ARG_obj(ip,1);
    struct sockaddr_in addr;
    if (0!=obj2addr(ip,&addr))
        THROW("invalid IP address");
    if (0!=connect(sock,(struct sockaddr*)&addr,sizeof addr)) 
        THROW(strerror(errno));
    return Undefined();
END

MODULE(net_posix,"net.posix")
    BIND("socket_tcp",posix_socket_tcp);
    BIND("bind",posix_bind);
    BIND("listen",posix_listen);
    BIND("accept",posix_accept);
    BIND("connect",posix_connect);
    //SET_int("O_TRUNC",O_TRUNC);
END_MODULE

// EOF - vim: ts=4 sw=4 noet
