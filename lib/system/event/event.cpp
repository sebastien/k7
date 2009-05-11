#ifdef WITH_EVENT
#include "macros.h"
#include <stdint.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/event_struct.h>
#include <event2/util.h>
#include <stdio.h>

using namespace v8;

FUNCTION(evbuf_constructor)
    evbuffer* buf = evbuffer_new();
    SET_INTERNAL(buf);
    return args.This();
END

FUNCTION(evbuf_push8)
    ARGINT(byte,"a byte to push");
    GET_INTERNAL(evbuffer*,buf);
    evbuffer_add(buf,&byte,1);
    return Undefined();
END

FUNCTION(evbuf_push_str)
    ARGSTR(str,"string to push");
    GET_INTERNAL(evbuffer*,buf);
    evbuffer_add(buf,*str,str.length());
    return Undefined();
END

FUNCTION(evbuf_pull8)
    uint8_t byte;
    GET_INTERNAL(evbuffer*,buf);
    if (1!=evbuffer_remove(buf,&byte,1))
        return Undefined();
    else
        return Integer::New((int)byte);
END

FUNCTION(evbuf_readln)
    GET_INTERNAL(evbuffer*,buf);
    char* str = evbuffer_readln(buf,NULL,EVBUFFER_EOL_CRLF);
    Handle<String> retstr(String::New(str));
    free(str);
    return handlescope.Close(retstr); // FIXME
END
    
FUNCTION(evbuf_read)
    ARGINT(fd,"file descriptor");
    ARGINT(toread,"max bytes read");
    GET_INTERNAL(evbuffer*,buf);
    size_t justread = evbuffer_read(buf,fd,toread);
    return Integer::New(justread);
END

FUNCTION(evbuf_write)
    ARGINT(fd,"file descriptor");
    GET_INTERNAL(evbuffer*,buf);
    size_t written = evbuffer_write(buf,fd);
    return Integer::New(written);
END

struct _ev_memo_t {
    event e;
    Persistent<Object> obj;
    Persistent<Function> func;
};
static const char* EV_MEMO_COOKIE = "_ev_memo_t";

/*OBJECT(event_EVENT,1,struct _ev_memo_t* ev)
    INTERNAL(0,ev);
    return self;
END*/

event_base * _base = NULL;

void _ev_cb (evutil_socket_t fd, short what, void *arg) {
    printf("callback!!!\n");
    HandleScope scope;
    struct _ev_memo_t * memo = (struct _ev_memo_t *) arg;
    Handle<Value> arr [] = {JS_int(fd),JS_int(what)};
    TryCatch tc;
    memo->func->Call(memo->obj,2,arr);
    if (tc.HasCaught()) {
        ReportException(&tc);
    }
    // FIXME free if not PERSISTENT
}

FUNCTION(le_event_add)
    ARGINT(fd,"file descriptor");
    ARGINT(flags,"EV_* flags");
    ARGOBJ(obj,"callback object");
    ARGFNC(ev_js_cb,"callback function");
    //ARG_str(comment,4);
    _ev_memo_t* memo = (_ev_memo_t*)malloc(sizeof(_ev_memo_t));
    memo->obj = Persistent<Object>::New(obj);
    memo->func = Persistent<Function>::New(ev_js_cb);
    event_assign(&(memo->e),_base,fd,flags,_ev_cb,memo);
    event_add(&(memo->e),NULL);
    printf("event added fd %i\n",fd);
    WRAP(wrapped,memo,EV_MEMO_COOKIE);
    printf("Wrapped!\n");
    RETURN_SCOPED(wrapped);
END

FUNCTION(le_event_del)
    ARGOBJ(evObj,"event wrapper object");
    //EXTERNAL(struct _ev_memo_t*,memo,evObj,0);
    UNWRAP(struct _ev_memo_t*,memo,evObj,EV_MEMO_COOKIE);
    event_del(&(memo->e));
    memo->obj.Dispose();
    memo->func.Dispose();
    free(memo);
END

FUNCTION(le_event_loop)
    ARGINT(millis,"milliseconds");
    struct timeval delay;
    delay.tv_sec = millis/1000;
    delay.tv_usec = (millis%1000)*1000;
    //event_base_loopexit(_base,&delay);
    event_base_loop(_base,0);
END

FUNCTION(le_make_socket_nonblocking)
    ARGINT(sock,"socket fd");
    int ret = evutil_make_socket_nonblocking(sock);
    return Integer::New(ret);
END

MODULE(system_event,"system.event")
    CLASS("Buffer")
        HAS_INTERNAL;
        CONSTRUCTOR(evbuf_constructor);
        METHOD("push8",evbuf_push8);
        METHOD("pull8",evbuf_pull8);
        METHOD("read",evbuf_read);
        METHOD("write",evbuf_write);
        METHOD("push_str",evbuf_push_str);
        METHOD("readln",evbuf_readln);
    END_CLASS
    WITH_MODULE
    BIND("add",le_event_add);
    BIND("del",le_event_del);
    BIND("loop",le_event_loop);
    BIND("make_socket_nonblocking",le_make_socket_nonblocking);
    SET_int("EV_TIMEOUT",EV_TIMEOUT);
    SET_int("EV_READ",EV_READ);
    SET_int("EV_WRITE",EV_WRITE);
    SET_int("EV_PERSIST",EV_PERSIST);
    SET_int("EV_SIGNAL",EV_SIGNAL);
    END
    struct event_config *cfg = event_config_new();
    event_config_require_features(cfg,EV_FEATURE_FDS);
    _base = event_base_new_with_config(cfg);
END_MODULE

#endif

