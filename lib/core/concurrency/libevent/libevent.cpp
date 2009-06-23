// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// ----------------------------------------------------------------------------
// Author            : Victor Grishchenko        <victor.grishchenko@gmail.com>
// ----------------------------------------------------------------------------
// Creation date     : 08-May-2009
// Last modification : 12-Jun-2009
// ----------------------------------------------------------------------------

// FIXME: Libevent uses threads, which is bad !
//
#ifdef WITH_LIBEVENT
#include "k7.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/event_struct.h>
#include <event2/util.h>

using namespace v8;

#define MODULE_NAME   "core.concurrency.libevent"
#define MODULE_STATIC  core_concurrency_libevent

void evbuf_free(Persistent<Value> watcher, void* parameter) {
	evbuffer_free((evbuffer*)parameter);
	watcher.Dispose();
}

FUNCTION(evbuf_new) {
	evbuffer* buf = evbuffer_new();
	RETURN_WATCHED(buf,evbuf_free);
} END

FUNCTION(evbuf_add8,PWRAP(evbuffer,buf),PINT(i)) {
	uint8_t i8 = i;
	evbuffer_add(buf,&i8,1);
	RETURN_UNDEF;
} END

FUNCTION(evbuf_add16,PWRAP(evbuffer,buf),PINT(i)) {
	uint16_t i16 = htons(i);
	evbuffer_add(buf,&i16,2);
	RETURN_UNDEF;
} END

FUNCTION(evbuf_add32,PWRAP(evbuffer,buf),PINT(i)) {
	uint32_t i32 = htonl(i);
	evbuffer_add(buf,&i32,4);
	RETURN_UNDEF;
} END

FUNCTION(evbuf_add,PWRAP(evbuffer,buf),PSTR(str)) {
	evbuffer_add(buf,*str,str.length());
	RETURN_UNDEF;
} END

FUNCTION(evbuf_remove8,PWRAP(evbuffer,buf)) {
	uint8_t byte;
	if (1!=evbuffer_remove(buf,&byte,1))
		RETURN_UNDEF;
	else
		RETURN_INT(byte);
} END

FUNCTION(evbuf_remove16,PWRAP(evbuffer,buf)) {
	uint16_t i;
	if (1!=evbuffer_remove(buf,&i,2))
		RETURN_UNDEF;
	else
		RETURN_INT(ntohs(i));
} END

FUNCTION(evbuf_remove32,PWRAP(evbuffer,buf)) {
	uint32_t i;
	if (1!=evbuffer_remove(buf,&i,1))
		RETURN_UNDEF;
	else
		RETURN_INT(htonl(i));
} END

FUNCTION(evbuf_remove,PWRAP(evbuffer,buf)) {
	size_t len = evbuffer_get_length(buf);
	char* b = (char*)evbuffer_pullup(buf,len);
	Handle<String> ret = String::New(b,len);
	evbuffer_drain(buf,len);
	RETURN_SCOPED(ret);
} END

FUNCTION(evbuf_readln,PWRAP(evbuffer,buf)) {
	char* str = evbuffer_readln(buf,NULL,EVBUFFER_EOL_CRLF);
	Handle<String> retstr(String::New(str));
	free(str);
	RETURN_SCOPED(retstr);
} END

FUNCTION(evbuf_read,PWRAP(evbuffer,buf),PINT(file_des),PINT(max_bytes)) {
	size_t justread = evbuffer_read(buf,file_des,max_bytes);
	RETURN_INT(justread);
} END

FUNCTION(evbuf_write,PWRAP(evbuffer,buf),PINT(file_des)) {
	size_t written = evbuffer_write(buf,file_des);
	RETURN_INT(written);
} END

struct _ev_memo_t {
	event e;
	Persistent<Object> obj;
	Persistent<Function> func;
};

event_base * _base = NULL;

void _ev_cb (evutil_socket_t fd, short what, void *arg) {
	HandleScope scope;
	struct _ev_memo_t * memo = (struct _ev_memo_t *) arg;
	Handle<Value> arr [] = {JS_int(fd),JS_int(what)};
	TryCatch tc;
	Handle<Value> ret = memo->func->Call(memo->obj,2,arr);
	if (tc.HasCaught()) {
		v8::String::AsciiValue exception(tc.Exception());
		fprintf(stderr,"Uncaught exception: %s\n",*exception);
	}
	if (tc.HasCaught() || !ret->IsBoolean() || !ret->ToBoolean()->Value()) {
		event_del(&(memo->e));
		memo->obj.Dispose();
		memo->func.Dispose();
	}
}

FUNCTION(le_event_add,PINT(fd),PINT(flags),POBJ(cbthis),PFUN(cbfun)) {
	_ev_memo_t* memo = (_ev_memo_t*)malloc(sizeof(_ev_memo_t));
	memo->obj = Persistent<Object>::New(cbthis);
	memo->func = Persistent<Function>::New(cbfun);
	event_assign(&(memo->e),_base,fd,flags|EV_PERSIST,_ev_cb,memo);
	event_add(&(memo->e),NULL);
	printf("event added fd %i\n",fd);
	RETURN_WRAPPED(memo);
} END

FUNCTION(le_event_del,PWRAP(_ev_memo_t,memo)) {
	event_del(&(memo->e));
	memo->obj.Dispose();
	memo->func.Dispose();
	free(memo);
} END

FUNCTION(le_event_loop,PINT(millis)) {
	struct timeval delay;
	delay.tv_sec = millis/1000;
	delay.tv_usec = (millis%1000)*1000;
	//event_base_loopexit(_base,&delay);
	event_base_loop(_base,0);
} END

FUNCTION(le_make_socket_nonblocking,PINT(sock)) {
	int ret = evutil_make_socket_nonblocking(sock);
	RETURN_INT(ret);
} END

MODULE {
	BIND("buffer_new",evbuf_new);
	//DESTRUCTOR(evbuf_destructor)
	BIND("buffer_add8",evbuf_add8);
	BIND("buffer_add16",evbuf_add16);
	BIND("buffer_add32",evbuf_add32);
	BIND("buffer_add",evbuf_add);
	BIND("buffer_remove8",evbuf_remove8);
	BIND("buffer_remove16",evbuf_remove16);
	BIND("buffer_remove32",evbuf_remove32);
	BIND("buffer_remove",evbuf_remove);
	BIND("buffer_read",evbuf_read);
	BIND("buffer_write",evbuf_write);
	BIND("buffer_readln",evbuf_readln);
	BIND("event_add",le_event_add);
	BIND("event_del",le_event_del);
	//BIND("event_loop",le_event_loop);
	BIND("make_socket_nonblocking",le_make_socket_nonblocking);
	SET_int("EV_TIMEOUT",EV_TIMEOUT);
	SET_int("EV_READ",EV_READ);
	SET_int("EV_WRITE",EV_WRITE);
	SET_int("EV_SIGNAL",EV_SIGNAL);
	struct event_config *cfg = event_config_new();
	event_config_require_features(cfg,EV_FEATURE_FDS);
	_base = event_base_new_with_config(cfg);
} END_MODULE

#endif
