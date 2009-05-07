#ifdef WITH_EVENT
#include "macros.h"
#include <stdint.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <stdio.h>

using namespace v8;

FUNCTION(event_echo)
    ARG_COUNT(1);
    ARG_str(str,0);
    printf("%s\n",*str);
    return Undefined();
END

FUNCTION(evbuf_constructor)
    ARG_COUNT(0);
    evbuffer* buf = evbuffer_new();
    SET_INTERNAL(buf);
    return args.This();
END

FUNCTION(evbuf_push8)
    ARG_COUNT(1);
    ARG_int(byte,0);
    GET_INTERNAL(evbuffer*,buf);
    evbuffer_add(buf,&byte,1);
    return Undefined();
END

FUNCTION(evbuf_pull8)
    ARG_COUNT(0);
    uint8_t byte;
    GET_INTERNAL(evbuffer*,buf);
    if (1!=evbuffer_remove(buf,&byte,1))
        return Undefined();
    else
        return Integer::New((int)byte);
END

FUNCTION(evbuf_read)
    ARG_COUNT(2);
    ARG_int(fd,0);
    ARG_int(toread,1);
    GET_INTERNAL(evbuffer*,buf);
    size_t justread = evbuffer_read(buf,fd,toread);
    return Integer::New(justread);
END

FUNCTION(evbuf_write)
    ARG_COUNT(1);
    ARG_int(fd,0);
    GET_INTERNAL(evbuffer*,buf);
    size_t written = evbuffer_write(buf,fd);
    return Integer::New(written);
END

MODULE(system_event,"system.event")
    CLASS("Buffer")
        HAS_INTERNAL;
        CONSTRUCTOR(evbuf_constructor);
        METHOD("push8",evbuf_push8);
        METHOD("pull8",evbuf_pull8);
        METHOD("read",evbuf_read);
        METHOD("write",evbuf_write);
    END_CLASS
END_MODULE

#endif

