// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
//                   : Tokuhiro Matsuno                    <tokuhirom@gmail.com>
// -----------------------------------------------------------------------------
// Creation date     : 13-Dec-2008
// Last modification : 08-May-2009
// -----------------------------------------------------------------------------

#ifdef WITH_FCGI

#include <k7.h>

#include <fcgiapp.h>
#include <string.h>

#define MODULE_NAME   "net.http.server.fcgi"
#define MODULE_STATIC net_http_server_fcgi

// ----------------------------------------------------------------------------
//
// MACROS & UTILITIES
//
// ----------------------------------------------------------------------------

inline static FCGX_Request *_get_handle(const v8::Arguments &args) {
	Local<Value> field = args.This()->GetInternalField(0);
	assert(field->IsExternal());
	FCGX_Request *req = reinterpret_cast<FCGX_Request *>(Handle<External>::Cast(field)->Value());
	assert(req);
	return req;
}

// ----------------------------------------------------------------------------
//
// FCGI CLASS
//
// ----------------------------------------------------------------------------

FUNCTION(FCGI_constructor)
{
	ARG_COUNT(0);
	if (FCGX_Init() != 0) {
		return v8::ThrowException(v8::String::New("failed FCGI_Init()"));
	}
	FCGX_Request *req = new FCGX_Request;
	FCGX_InitRequest(req, 0, 0);
	
	args.This()->SetInternalField(0, External::New((void*)req));
	return args.This();
}
END

FUNCTION(FCGI_accept)
{
	ARG_COUNT(0);
	FCGX_Request *req = _get_handle(args);
	return Int32::New( FCGX_Accept_r(req) );
}
END

FUNCTION(FCGI_getEnv)
{
	ARG_COUNT(0);
	
	FCGX_Request *req = _get_handle(args);
	
	v8::Handle<v8::Object> env = v8::Object::New();
	for (int i = 0; req->envp[i]; i ++) {
		char *str = new char[strlen(req->envp[i])];
		strcpy(str, req->envp[i]);
		int j, l = strlen(req->envp[i]);
		for (j = 0; j < l && str[j] != '='; j ++);
		str[j] = '\0';
		
		
		// FCGX_PutStr(req->envp[i], strlen(req->envp[i]), req->err);
		// FCGX_PutStr("\n", 1, req->err);
		env->Set(JS_str(str),JS_str(str + j + 1));
	}
	return env;
}
END



FUNCTION(FCGI_getRawRequest)
{
	ARG_COUNT(0);
	
	v8::Handle<v8::String> ret;
	
	FCGX_Request *req = _get_handle(args);
	
	// stupid FCGX gives us strings that aren't \0 terminated, so
	// the c string functions don't work.  that's why the 255/256
	// business, and the manually putting the \0s there.
	
	char *rawReq = new char[256], *tmp, *tmp2;
	int i = FCGX_GetStr(rawReq, 255, req->in);
	int j = i;
	rawReq[i] = '\0';
	while (i >= 255) {
		tmp = new char[ j + 1 ];
		strcpy(tmp, rawReq);
		tmp2 = new char[256];
		i = FCGX_GetStr(tmp2, 255, req->in);
		tmp2[i + 1] = '\0';
		if (i > 0) {
			j += i;
			delete[] rawReq;
			rawReq = new char[j + 1];
			strcpy(rawReq, tmp);
			strcat(rawReq, tmp2);
		}
		delete[] tmp;
		delete[] tmp2;
	}
	
	ret = JS_str(rawReq);
	delete[] rawReq;
	return ret;
}
END

FUNCTION(FCGI_putstr)
{
	ARG_COUNT(1);
	ARG_str(str, 0);
	FCGX_Request *req = _get_handle(args);
	FCGX_PutStr(*str, str.length(), req->out);
	return Undefined();
}
END

FUNCTION(FCGI_errorLog)
{
	ARG_COUNT(1);
	ARG_str(str, 0);
	FCGX_Request *req = _get_handle(args);
	FCGX_PutStr(*str, str.length(), req->err);
	return Undefined();
}
END

FUNCTION(FCGI_free)
{
	ARG_COUNT(0);
	FCGX_Request *req = _get_handle(args);
	FCGX_Free(req, 1);
	return Undefined();
}
END

MODULE
{
	CLASS("FCGI")
	{
		INTERNAL_FIELDS(1);
		CONSTRUCTOR(FCGI_constructor);
		METHOD("accept", FCGI_accept);
		METHOD("putstr", FCGI_putstr);
		METHOD("errorLog", FCGI_errorLog);
		METHOD("free",   FCGI_free);
		METHOD("getEnv", FCGI_getEnv);
		METHOD("getRawRequest", FCGI_getRawRequest);
	}
	END_CLASS
}
END_MODULE

#endif
// EOF
