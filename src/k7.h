// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
// Author            : Isaac Schulter.                           <i@foohack.com>
// ----------------------------------------------------------------------------
// Creation date     : 27-Sep-2008
// Last modification : 23-Jun-2009
// ----------------------------------------------------------------------------

#ifndef __K7_H__
#define __K7_H__

#include <v8.h>
#include "macros.h"
#include <assert.h>

using namespace v8;

namespace k7 {

	void           setup   (Handle<Object> global,int argc, char** argv, char** env);
	void           trace   (const v8::TryCatch* try_catch);
	void           trace   (Handle<Message> message);
	void           onMessage (Handle<Message> message, Handle<Value> data);

	bool           execute (const char* source);
	bool           execute (const char* source, const char* fromFileName);
	bool           execute (Handle<String> source);
	bool           execute (Handle<String> source, Handle<Value> fromFileName);

	Handle<Value>  eval    (const char* source);
	Handle<Value>  eval    (const char* source, const char* fromFileName);
	Handle<Value>  eval    (Handle<String> source);
	Handle<Value>  eval    (Handle<String> source, Handle<Value> fromFileName);

	Handle<Value>  read    (const char* path);

	Handle<Object> module  (const char* fullName);
	Handle<Object> module  (Handle<Object> parent, const char* moduleName, const char* fullName);

	Handle<Value>  dynload (Handle<Object> parent, const char* modulePath);

	int       main    (int argc, char **argv, char **env);

}

#endif
// EOF - vim: ts=4 sw=4 noet
