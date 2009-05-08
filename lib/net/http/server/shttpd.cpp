// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
//                   : Tokuhiro Matsuno                    <tokuhirom@gmail.com>
// -----------------------------------------------------------------------------
// Creation date     : 29-Sep-2008
// Last modification : 08-May-2009
// -----------------------------------------------------------------------------

#include <k7.h>

#include <cassert>
#include <cstring>
#include <string>
#include <sstream>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/wait.h>
#include "shttpd/src/shttpd.h"

LINK_TO(libshttpd.a)

#define SHTTPD_POLL_DELAY 10
#define MODULE_NAME   "net.http.server.shttpd"
#define MODULE_STATIC  net_http_server_shttpd

// ----------------------------------------------------------------------------
//
// MACROS & UTILITIES
//
// ----------------------------------------------------------------------------

template <class T> static inline T * handle(const v8::Arguments & args, int index) {
	v8::Local<v8::Value> field = args.This()->GetInternalField(index);
	assert(field->IsExternal());
	T* ret = reinterpret_cast<T *>(v8::Handle<v8::External>::Cast(field)->Value());
	assert(ret);
	return ret;
}

static inline v8::Handle<v8::Object> shttpd_namespace() {
		return v8::Context::GetCurrent()->Global()
					->Get(v8::String::New("net"))->ToObject()
					->Get(v8::String::New("http"))->ToObject()
					->Get(v8::String::New("server"))->ToObject()
					->Get(v8::String::New("shttpd"))->ToObject();
}

#define ARG_shttpd_arg(arg) shttpd_arg* arg = handle<shttpd_arg>( args, 0 );
#define ARG_shttpd_ctx(arg) shttpd_ctx* arg = handle<shttpd_ctx>(args, 0);
#define HANDLERS_GLOBAL "_Handlers"

// ----------------------------------------------------------------------------
//
// ARGUMENT PROTOTYPE
//
// ----------------------------------------------------------------------------

FUNCTION(Request_print)
{
	ARG_COUNT(1);
	ARG_utf8(str,0);
	ARG_shttpd_arg(arg);
	shttpd_printf(arg, "%s", *str);
	return args.This();
}
END

FUNCTION(Request_setFlags)
{
	ARG_COUNT(1);
	ARG_int(flags,0);
	ARG_shttpd_arg(arg);
	arg->flags = flags;
	return args.This();
}
END

FUNCTION(Request_getEnv)
{
	ARG_COUNT(1);
	ARG_utf8(name,0);
	ARG_shttpd_arg(arg);
	const char *result = shttpd_get_env(arg, *name);
	if (result == NULL)
	    return JS_null;
	return JS_str(result);
}
END

FUNCTION(Request_getVar)
{
	ARG_BETWEEN(1,2);
	ARG_utf8(name,0);
	ARG_shttpd_arg(arg);
	uint32_t bufsize = args.Length() == 2 ? args[1]->Uint32Value() : 1024*1024;
	char *buf = new char [bufsize];
	shttpd_get_var(*name, arg->in.buf, arg->in.len, buf, bufsize);
	v8::Handle<v8::String> ret = v8::String::New(buf);
	delete [] buf;
	return ret;
}
END

FUNCTION(Request_getHeader)
{
	ARG_COUNT(1)
	ARG_utf8(name,0);
	ARG_shttpd_arg(arg);
    const char *result = shttpd_get_header(arg, *name);
	return result ? JS_str(result) : JS_null;
}
END

// ----------------------------------------------------------------------------
//
// SERVER PROTOTYPE
//
// ----------------------------------------------------------------------------

FUNCTION(Server_constructor)
{
	ARG_COUNT(1);
	ARG_utf8(port,0);
		char * argv[6];
		argv[0] = (char*)"k7";
		argv[1] = (char*)"-dir_list";
		argv[2] = (char*)"no";
		argv[3] = (char*)"-ports";
		argv[4] = *port;
		argv[5] = NULL;
		shttpd_ctx *ctx = shttpd_init(6, argv);
		assert(ctx);
		THIS->SetInternalField(0, v8::External::New((void*)ctx));
	return THIS;
}
END

FUNCTION(Server_close)
{
	ARG_COUNT(0)
	ARG_shttpd_ctx(ctx)
		shttpd_fini(ctx);
	return THIS;
}
END

FUNCTION(Server_setOption)
{
	ARG_COUNT(2)
	ARG_shttpd_ctx(ctx)
	ARG_utf8(key,0);
	ARG_utf8(val,1);
		shttpd_set_option(ctx, *key, *val);
	return THIS;
}
END

void Server__onRequest(struct shttpd_arg *_arg) {
	v8::HandleScope hs;
	// We retrieve the handler function from the shared HANDLERS_GLOBAL variable
	v8::Handle<v8::Function> handler = v8::Handle<v8::Function>::Cast(
		shttpd_namespace()
		->Get(JS_str(HANDLERS_GLOBAL))
		->ToObject()
		->Get(JS_str(static_cast<char*>(_arg->user_data)))
	);

	// We wrap the arguments into a new Argument object
	// FIXME: Why a cast to a function .
	v8::Handle<v8::Object> arg = v8::Handle<v8::Function>::Cast(
		shttpd_namespace()->Get(JS_str("Request"))
	)->NewInstance();
	arg->SetInternalField(0, v8::External::New(_arg));

	v8::Handle<v8::Value> funcargs[1];
	funcargs[0] = arg;

	// We call the handler
	handler->Call(v8::Context::GetCurrent()->Global(), 1, funcargs);
}

FUNCTION(Server_registerURI)
{
	ARG_COUNT(2);
	ARG_shttpd_ctx(ctx);
	ARG_utf8(uri,0);
		std::ostringstream tmp;
		tmp << "RU";
		tmp << *uri;
		tmp << rand(); // make unique...
		char * hashkey = new char [tmp.str().length() + 1];
		strcpy(hashkey, tmp.str().c_str());
		shttpd_namespace()->Get(
			JS_str(HANDLERS_GLOBAL))
			->ToObject()
			->Set(JS_str(hashkey),
			args[1]
		);
		shttpd_register_uri(ctx, *uri, Server__onRequest, hashkey);
	return THIS;
}
END

FUNCTION(Server_handleError)
{
	ARG_COUNT(2)
	ARG_shttpd_ctx(ctx);
	ARG_int(status,0);
		std::ostringstream tmp;
		tmp << "HE";
		tmp << status;
		tmp << rand(); // make unique...
		char * hashkey = new char [tmp.str().length() + 1];
		strcpy(hashkey, tmp.str().c_str());
		shttpd_namespace()->Get(JS_str(HANDLERS_GLOBAL))
			->ToObject()
			->Set(JS_str(hashkey), args[1]);
		shttpd_handle_error(ctx, status, Server__onRequest, hashkey);
	return THIS;
}
END

FUNCTION(Server_processRequests)
{
	ARG_BETWEEN(0,1);
	int delay=SHTTPD_POLL_DELAY;
	if (ARGC==1) { ARG_int(d,0) ; delay = d; }
	ARG_shttpd_ctx(ctx);
	shttpd_poll(ctx, delay);
	return THIS;
}
END

// ----------------------------------------------------------------------------
//
// MODULE DECLARATION
//
// ----------------------------------------------------------------------------

MODULE
{

	SET("VERSION",          JS_str(shttpd_version()));
	SET("END_OF_OUTPUT",    JS_int(SHTTPD_END_OF_OUTPUT)) 
	SET("CONNECTION_ERROR", JS_int(SHTTPD_CONNECTION_ERROR)) 
	SET("MORE_POST_DATA",   JS_int(SHTTPD_MORE_POST_DATA)) 
	SET("POST_BUFFER_FULL", JS_int(SHTTPD_POST_BUFFER_FULL)) 
	SET("SSI_EVAL_TRUE",    JS_int(SHTTPD_SSI_EVAL_TRUE)) 
	SET("SUSPEND",          JS_int(SHTTPD_SUSPEND)) 

	/*
	PROTOTYPE("Server")
		METHOD("setOption", Server_setOption)
		METHOD("close",     Server_close)
		METHOD("startLoop", Server_startLoop)
	*/
	
	{
		v8::Handle<v8::FunctionTemplate> ft = v8::FunctionTemplate::New(Server_constructor);
		v8::Handle<v8::ObjectTemplate>   ot = ft->InstanceTemplate();
		ot->Set("setOption",      v8::FunctionTemplate::New(Server_setOption));
		ot->Set("close",          v8::FunctionTemplate::New(Server_close));
		ot->Set("processRequests",v8::FunctionTemplate::New(Server_processRequests));
		ot->Set("registerURI",    v8::FunctionTemplate::New(Server_registerURI));
		ot->Set("handleError",    v8::FunctionTemplate::New(Server_handleError));
		ot->SetInternalFieldCount(1);
		__module__->Set(
			v8::String::New("Server"),
			ft->GetFunction(),
			v8::PropertyAttribute(v8::ReadOnly | v8::DontDelete)
		);
	}

	{
		v8::Handle<v8::FunctionTemplate> ft = v8::FunctionTemplate::New();
		v8::Handle<v8::ObjectTemplate>   ot = ft->InstanceTemplate();
		ot->Set(JS_str("print"),     v8::FunctionTemplate::New(Request_print)->GetFunction());
		ot->Set(JS_str("getEnv"),    v8::FunctionTemplate::New(Request_getEnv)->GetFunction());
		ot->Set(JS_str("getVar"),    v8::FunctionTemplate::New(Request_getVar)->GetFunction());
		ot->Set(JS_str("getHeader"), v8::FunctionTemplate::New(Request_getHeader)->GetFunction());
		ot->Set(JS_str("setFlags"),  v8::FunctionTemplate::New(Request_setFlags)->GetFunction());
		ot->SetInternalFieldCount(1);
		__module__->Set(
			JS_str("Request"),
			ft->GetFunction(),
			v8::PropertyAttribute(v8::ReadOnly | v8::DontDelete)
		);
	}

	__module__->Set(
		JS_str(HANDLERS_GLOBAL),
		v8::Object::New()
	);

}
END_MODULE

// EOF - vim: ts=4 sw=4 noet
