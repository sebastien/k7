// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
// ----------------------------------------------------------------------------
// Creation date     : 29-Sep-2008
// Last modification : 29-Sep-2008
// ---------------------------------------------------------------------------

template <class T>
inline T * handle(const v8::Arguments & args, int index) {
    v8::Local<v8::Value> field = args.This()->GetInternalField(index);
    assert(field->IsExternal());
    T* ret = reinterpret_cast<T *>(v8::Handle<v8::External>::Cast(field)->Value());
    assert(ret);
    return ret;
}

static inline v8::Handle<v8::Object> shttpd_namespace() {
        return v8::Context::GetCurrent()->Global()
                      ->Get(v8::String::New("org"))->ToObject()
                      ->Get(v8::String::New("coderepos"))->ToObject()
                      ->Get(v8::String::New("shttpd"))->ToObject();
}

#define ARG_shttpd_arg(arg) EXTERNAL(shttpd_arg*,arg,THIS,0);
#define ARG_shttpd_ctx(arg) EXTERNAL(shttpd_ctx*,arg,THIS,0);

/////////////////////////////////////////////////////////////
// handler

FUNCTION(print)
	ARG_COUNT(1);
	ARG_utf8(str,0);
	EXTERNAL(shttpd_arg*,arg,THIS,0);
	shttpd_printf(arg, "%s", *str);
	return THIS;
END

FUNCTION(setFlags)
	ARG_COUNT(1)
	ARG_int(flags,0)
	EXTERNAL(shttpd_arg*,arg,THIS,0);
	arg->flags = flags;
	return THIS;
END

FUNCTION(getEnv)
	ARG_COUNT(1);
	ARG_utf8(str,0);
	EXTERNAL(shttpd_arg*,arg,THIS,0);
	shttpd_printf(arg, "%s", *str);
	return JS_str(shttpd_get_env(arg, *name));
END

FUNCTION(getVar)
	ARG_BETWEEN(1,2);
	ARG_utf8(str,0);
	EXTERNAL(shttpd_arg*,arg,THIS,0);
	uint32_t bufsize = args.Length() == 2 ? args[1]->Uint32Value() : 1024*1024;
	char *buf = new char [bufsize];
	shttpd_get_var(*name, arg->in.buf, arg->in.len, buf, bufsize);
	Handle<String> ret = String::New(buf);
	delete [] buf;
	return ret;
END

FUNCTION(getHeader)
	ARG_COUNT(1)
	ARG_utf8(str,0);
	ARG_shttpd(arg)
	return JS_str(shttpd_get_header(arg, *name));
END

void _callback(struct shttpd_arg *_arg) {
	HandleScope hs;

	Handle<Function> func = Handle<Function>::Cast( shttpd_namespace()->Get(String::New("__HandlerStorage"))->ToObject()->Get(String::New(static_cast<char*>(_arg->user_data))) );

	Handle<Object> arg = Handle<Function>::Cast( shttpd_namespace()->Get(String::New("Arg")) )->NewInstance();
	arg->SetInternalField(0, External::New(_arg));

	Handle<Value> funcargs[1];
	funcargs[0] = arg;
	func->Call(Context::GetCurrent()->Global(), 1, funcargs);
}

/////////////////////////////////////////////////////////////
// instance methods

FUNCTION(Server_init)
	ARG_COUNT(1);
	ARG_utf8(port,0);
		char * argv[6] = [
			"k7",
			"-dir_list",
			"no",
			"-ports",
			*port,
			NULL,
		];
		shttpd_ctx *ctx = shttpd_init(6, argv);
		assert(ctx);
		THIS->SetInternalField(0, External::New((void*)ctx));
	return THIS
END

FUNCTION(Server_close)
	ARG_COUNT(0)
	ARG_shttpd_ctx(ctx,0)
		shttpd_fini(ctx);
	return THIS;
END

FUNCTION(Server_setOption)
	ARG_COUNT(2)
	ARG_shttpd_ctx(ctx,0)
	ARG_ut8(key,0);
	ARG_ut8(val,1);
		shttpd_set_option(ctx, *key, *val);
	return THIS;
END

FUNCTION(Server_registerURI)
	ARG_COUNT(2);
	ARG_shttpd_ctx(ctx);
	ARG_utf8(uri,0);
		std::ostringstream tmp;
		tmp << "RU";
		tmp << *uri;
		tmp << rand(); // make unique...
		char * hashkey = new char [tmp.str().length() + 1];
		strcpy(hashkey, tmp.str().c_str());
		shttpd_namespace()->Get(String::New("__HandlerStorage"))->ToObject()->Set(String::New(hashkey), args[1]);
		shttpd_register_uri(ctx, *uri, _callback, hashkey);
	return THIS;
END

FUNCTION(Server_handleError)
	ARG_COUNT(2)
	ARG_shttpd_ctx(ctx)
	ARG_int(status,0)
		std::ostringstream tmp;
		tmp << "HE";
		tmp << status;
		tmp << rand(); // make unique...
		char * hashkey = new char [tmp.str().length() + 1];
		strcpy(hashkey, tmp.str().c_str());
		shttpd_namespace()->Get(String::New("__HandlerStorage"))->ToObject()->Set(String::New(hashkey), args[1]);
		shttpd_handle_error(ctx, status, _callback, hashkey);
	return THIS
END

FUNCTION(Server_startLoop)
static v8::Handle<v8::Value> _start_loop(const v8::Arguments& args) {
	ARG_COUNT(0);
	ARG_shttpd_ctx(ctx);
		for (;;) {
			shttpd_poll(ctx, 1000);
		}
	return THIS;
END

/////////////////////////////////////////////////////////////
// initialize

INIT(net_http_server_shttpd,"net.http.server.shttpd")

	Handle<FunctionTemplate> ft = FunctionTemplate::New(_init);
	VALUE("VERSION",          JS_str(shttpd_version());
	VALUE("END_OF_OUTPUT",    JS_int(SHTTPD_END_OF_OUTPUT)) 
	VALUE("CONNECTION_ERROR", JS_int(SHTTPD_CONNECTION_ERROR)) 
	VALUE("MORE_POST_DATA",   JS_int(SHTTPD_MORE_POST_DATA)) 
	VALUE("POST_BUFFER_FULL", JS_int(SHTTPD_POST_BUFFER_FULL)) 
	VALUE("SSI_EVAL_TRUE",    JS_int(SHTTPD_SSI_EVAL_TRUE)) 
	VALUE("SUSPEND",          JS_int(SUSPEND)) 

	PROTOTYPE("Server")
		METHOD("setOption", Server_setOption)
		METHOD("close",     Server_close)
		
	Handle<ObjectTemplate>   ot = ft->InstanceTemplate();
		ot->Set("SetOption",   FunctionTemplate::New(_set_option));
		ot->Set("Close",       FunctionTemplate::New(_close));
		ot->Set("StartLoop",   FunctionTemplate::New(_start_loop));
		ot->Set("RegisterURI", FunctionTemplate::New(_register_uri));
		ot->Set("HandleError", FunctionTemplate::New(_handle_error));
		ot->SetInternalFieldCount(1);
		target->Set(
			String::New("SHTTPD"),
			ft->GetFunction(),
			PropertyAttribute(ReadOnly | DontDelete)
		);

	Handle<FunctionTemplate> ft = FunctionTemplate::New();
	Handle<ObjectTemplate>   ot = ft->InstanceTemplate();
	ot->Set(String::New("Print"),     FunctionTemplate::New(_arg_print)->GetFunction());
	ot->Set(String::New("GetEnv"),    FunctionTemplate::New(_arg_get_env)->GetFunction());
	ot->Set(String::New("GetVar"),    FunctionTemplate::New(_arg_get_var)->GetFunction());
	ot->Set(String::New("GetHeader"), FunctionTemplate::New(_arg_get_header)->GetFunction());
	ot->Set(String::New("SetFlags"),  FunctionTemplate::New(_arg_set_flags)->GetFunction());
	ot->SetInternalFieldCount(1);
	target->Set(
		String::New("Arg"),
		ft->GetFunction(),
		PropertyAttribute(ReadOnly | DontDelete)
	);

	target->Set(
		String::New("__HandlerStorage"),
		Object::New()
	);

    return module;
END

// EOF - vim: ts=4 sw=4 noet
