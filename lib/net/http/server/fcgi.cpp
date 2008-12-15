// ----------------------------------------------------------------------------
// Project		   : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author			: Sebastien Pierre				   <sebastien@type-z.org>
//				   : Tokuhiro Matsuno					<tokuhirom@gmail.com>
// -----------------------------------------------------------------------------
// Creation date	 : 13-Dec-2008
// Last modification : 13-Dec-2008
// -----------------------------------------------------------------------------


#include <k7.h>

#include <fcgiapp.h>

using namespace v8;

// ----------------------------------------------------------------------------
//
// API
//
// ----------------------------------------------------------------------------

/*
START_API
@module net.http.server.fcgi

@class FCGI

	@constructor

	@abstract @method accept

	@abstract @method putstr

	@abstract @method free

@end
END_API
*/

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
	ARG_COUNT(0);
	if (FCGX_Init() != 0) {
		return v8::ThrowException(v8::String::New("failed FCGI_Init()"));
	}
	FCGX_Request *req = new FCGX_Request;
	FCGX_InitRequest(req, 0, 0);
	args.This()->SetInternalField(0, External::New((void*)req));
	return args.This();
END

FUNCTION(FCGI_accept)
	ARG_COUNT(0);
	FCGX_Request *req = _get_handle(args);
	return Int32::New( FCGX_Accept_r(req) );
END

FUNCTION(FCGI_putstr)
	ARG_COUNT(1);
	ARG_str(str, 0);
	FCGX_Request *req = _get_handle(args);
	FCGX_PutStr(*str, str.length(), req->out);
	return Undefined();
END

FUNCTION(FCGI_free)
	ARG_COUNT(0);
	FCGX_Request *req = _get_handle(args);
	FCGX_Free(req, 1);
	return Undefined();
END

MODULE(net_http_server_fcgi,"net.http.server.fcgi")
	CLASS("FCGI")
		INTERNAL_FIELDS(1);
		CONSTRUCTOR(FCGI_constructor);
		METHOD("accept", FCGI_accept);
		METHOD("putstr", FCGI_putstr);
		METHOD("free",   FCGI_free);
	END_CLASS
END_MODULE

// EOF
