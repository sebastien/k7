// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Victor Grishchenko        <victor.grishchenko@gmail.com>
//                     Sébastien Pierre                  <sebastien@type-z.org>
// ----------------------------------------------------------------------------
// Creation date     : 07-May-2009
// Last modification : 11-May-2009
// ----------------------------------------------------------------------------

#include "k7.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

using namespace v8;

#define MODULE_NAME  "net.posix"
#define MODULE_STATIC net_posix

int obj2addr (Handle<Object> obj, struct sockaddr_in* addr) {
	HandleScope scope;
	String::AsciiValue a (obj->Get(JS_str("addr")));
	Handle<Value> p = obj->Get(JS_str("port"));
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
	obj->Set(JS_str("port"),JS_int(ntohs(addr->sin_port)));
	char str[32];
	inet_ntop(AF_INET,&(addr->sin_addr),str,sizeof(sockaddr_in));
	obj->Set(JS_str("addr"),String::New(str));
	return scope.Close(obj);
}

FUNCTION(posix_socket_tcp)
	ARG_COUNT(0);
	int sock = socket(AF_INET,SOCK_STREAM,0);
	return JS_int(sock);
END

FUNCTION(posix_bind)
	ARG_COUNT(2);
	ARG_int(sock,0);
	ARG_obj(ip,1);
	struct sockaddr_in addr;
	if (0!=obj2addr(ip,&addr))
		JS_ERROR("invalid IP address");
	if (0!=bind(sock,(struct sockaddr*)&addr,sizeof addr)) 
		JS_ERROR(strerror(errno));
	return Undefined();
END

FUNCTION(posix_listen)
	ARG_COUNT(1);
	ARG_int(sock,0);
	if (listen(sock,8)==-1) 
		JS_ERROR(strerror(errno));
	return Undefined();
END

FUNCTION(posix_accept)
	ARG_COUNT(1);
	ARG_int(sock,0);
	struct sockaddr_in addr;
	socklen_t len;
	int newsock = accept(sock,(struct sockaddr*)&addr,&len);
	if (-1==newsock)
		JS_ERROR(strerror(errno));
	Handle<Object> addrobj = addr2obj(&addr);
	addrobj->Set(JS_str("sock"),JS_int(newsock));
	return addrobj;
END

FUNCTION(posix_connect)
	ARG_COUNT(2);
	ARG_int(sock,0);
	ARG_obj(ip,1);
	struct sockaddr_in addr;
	if (0!=obj2addr(ip,&addr))
		JS_ERROR("invalid IP address");
	if (0!=connect(sock,(struct sockaddr*)&addr,sizeof addr)) 
		JS_ERROR(strerror(errno));
	return Undefined();
END

FUNCTION(posix_net_close)
	ARG_COUNT(1);
	ARG_int(sock,0);
	close(sock);
	return Undefined();
END

MODULE
	BIND("socket_tcp",posix_socket_tcp);
	BIND("bind",posix_bind);
	BIND("listen",posix_listen);
	BIND("accept",posix_accept);
	BIND("connect",posix_connect);
	BIND("close",posix_net_close);
	//SET_int("O_TRUNC",O_TRUNC);
END_MODULE

// EOF - vim: ts=4 sw=4 noet
