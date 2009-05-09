// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
// ----------------------------------------------------------------------------
// Creation date     : 27-Sep-2008
// Last modification : 19-Mar-2009
// ----------------------------------------------------------------------------

#include "macros.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <string>

// TODO: Add proper error handling
// #define MODULE "system.posix"
using namespace v8;

OBJECT(posix_FILE,1,FILE* file)
	INTERNAL(0,file);
	return self;
END

FUNCTION(posix_time)
	ARG_COUNT(0)
	return JS_int(time(NULL));
END

FUNCTION(posix_fopen)
	ARG_COUNT(2)
	ARG_utf8(path,0);
	ARG_utf8(mode,1);
	FILE* fd = fopen(*path,*mode);
	if (fd == NULL)
		return JS_null;
	return posix_FILE(fd);
END

FUNCTION(posix_fclose)
	ARG_COUNT(1);
	ARG_obj(fileObj,0);
	EXTERNAL(FILE*,file,fileObj,0);
	return JS_int(fclose(file));
END

FUNCTION(posix_popen)
	ARG_COUNT(2)
	ARG_utf8(path,0);
	ARG_utf8(type,1);
	FILE* fd = popen(*path,*type);
	if (fd == NULL)
		return JS_null;
	return posix_FILE(fd);
END

FUNCTION(posix_pclose)
	ARG_COUNT(1);
	ARG_obj(fileObj,0);
	EXTERNAL(FILE*,file,fileObj,0);
	return JS_int(pclose(file));
END

FUNCTION(posix_system)
	ARG_COUNT(1);
	ARG_utf8(command,0);
	return JS_int(system(*command));
END

FUNCTION(posix_fwrite)
	ARG_utf8(data,0);
	ARG_int(size,1);
	ARG_int(nmemb,2);
	ARG_obj(fileObj,3);
	EXTERNAL(FILE*,file,fileObj,0);
	return JS_int(fwrite(*data,size,nmemb,file));
END

FUNCTION(posix_fread)
	ARG_COUNT(3);
	ARG_int(size,0);
	ARG_int(nmemb,1);
	ARG_obj(fileObj,2);
	EXTERNAL(FILE*,file,fileObj,0);
	if (size < 0) {
		return v8::ThrowException(JS_str("Exception: invalid bufsize"));
	}
	char *buf   = new char[size*nmemb + 1024];
	size_t read = fread(buf, size, nmemb, file);
	v8::Handle<v8::String> strbuf = JS_str2(buf, read);
	delete [] buf;
	return strbuf;
END

FUNCTION(posix_feof)
{
	ARG_obj(fileObj, 1);
	EXTERNAL(FILE*,file,fileObj,0);
	return Boolean::New(feof(file));
}
END
	
FUNCTION(posix_readfile)
	STUB
END

FUNCTION(posix_writefile)
{
	ARG_COUNT(2);
	ARG_utf8(name, 0);
	ARG_utf8(data, 1);
	
	if (*name == NULL) {
		return v8::ThrowException(v8::String::New("Invalid filename"));
	}

	FILE* file = fopen(*name, "wb");
	if (file == NULL) {
		return v8::ThrowException(v8::String::New("Could not open file for writing."));
	}
	int result = fwrite(*data,strlen(*data),1,file);
	fclose(file);
	
	return JS_int(result);
	
}
END

// See this good tutorial for the pthreads API
// https://computing.llnl.gov/tutorials/pthreads/

OBJECT(posix_PTHREAD,1,pthread_t* thread)
	INTERNAL(0,thread)
	return self;
END

// NOTE: This is WIP code that will be moved to a "task" module using libtask API
void* posix_pthread_create_callback(void* context) {
	//ARG_COUNT(1);
	//ARG_obj(callback_context,0);
	printf("NEW THREAD STARTED...\n");

	Locker locker;
	HandleScope HandleScope;

	Object* context_obj = reinterpret_cast<Object*>(context);

	Handle<Function> callback_v = Handle<Function>::Cast(context_obj->Get(JS_str("callback")));
	callback_v->Call(v8::Context::GetCurrent()->Global(), 0, NULL);

	// TODO: We should release the callback context
}

FUNCTION(posix_pthread_create)
	ARG_COUNT(2);
	//Handle<Function> callback = Handle<Function>::Cast(args[(0)]);
	ARG_fn(callback,0);
	ARG_obj(arguments, 1);
	//Persistent<Arguments> thread_arguments = args;
	// FIXME: This won't work, of course

	pthread_t* thread = (pthread_t*) malloc(sizeof(pthread_t));
	Persistent<Object> result = Persistent<Object>::New(posix_PTHREAD(thread));
	result->Set(JS_str("callback"),callback);
	/* SEGFAULT HERE
	result->Set(JS_str("arguments"),arguments);
	*/
	//pthread_create(thread, NULL, posix_pthread_create_callback, (void*)*result);
	posix_pthread_create_callback((void*)*result);
	return result;
END

FUNCTION(posix_open)
    ARG_COUNT(2);
    ARG_str(name,0);
    ARG_int(mode,1);
    int fd = open(*name,mode,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (fd<0)
        return ThrowException(String::New(strerror(errno)));
    else
        return Integer::New(fd);
END

FUNCTION(posix_close)
    ARG_COUNT(1);
    ARG_int(fd,0);
    close(fd);
    return Undefined();
END

MODULE(system_posix,"system.posix")
	// FIXME: When I set the module 'time' slot to a string, accessing the slot
	// from JavaScript works, but when I BIND it to the posix_time function, the
	// JavaScript returns undefined. Even worse, the next BIND has no effect.
	//module->Set(v8::String::New("time"), JS_str("system.posix.time is a string"));
	BIND("time",      posix_time);
	BIND("fopen",     posix_fopen);
	BIND("fwrite",    posix_fwrite);
	BIND("fread",     posix_fread);
	BIND("fclose",    posix_fclose);
	BIND("writefile", posix_writefile);
	BIND("readfile",  posix_readfile);
	BIND("feof",      posix_feof);
	BIND("popen",     posix_popen);
	BIND("pclose",    posix_pclose);
	BIND("system",    posix_system);
	//BIND("pthread_create",  posix_pthread_create);
    BIND("open",posix_open);
    BIND("close",posix_close);
    SET_int("O_RDWR",O_RDWR);
    SET_int("O_RDONLY",O_RDONLY);
    SET_int("O_WRONLY",O_WRONLY);
    SET_int("O_NONBLOCK",O_NONBLOCK);
    SET_int("O_CREAT",O_CREAT);
    SET_int("O_TRUNC",O_TRUNC);
END_MODULE

// EOF - vim: ts=4 sw=4 noet
