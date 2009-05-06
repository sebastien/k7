// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
// ----------------------------------------------------------------------------
// Creation date     : 27-Sep-2008
// Last modification : 09-Apr-2009
// ----------------------------------------------------------------------------

#ifndef __K7_MACROS__
#define __K7_MACROS__

#include <k7.h>

using namespace v8;

// ----------------------------------------------------------------------------
//
// TYPE CONVERSION
//
// ----------------------------------------------------------------------------

/**
 * These macros are just shorthand to creat V8/JavaScript values that can be
 * passed to the V8 API or returned to the JavaScript environment */
#define JS_str(s)                      v8::String::New(s)
#define JS_str2(s,c)                   v8::String::New(s,c)
#define JS_int(s)                      v8::Integer::New(s)
#define JS_undefined                   v8::Undefined()
#define JS_null                        v8::Null()
#define JSOBJ_set(target,slot,value)   target->Set(JS_str(slot),value)
#define JS_fn(f)                       v8::FunctionTemplate::New(f)->GetFunction()
#define V8_FT(f)                       v8::FunctionTemplate::New(f)
#define JS_obj(o)                      v8::Object::New(o)
#define JS_bool(b)                     v8::Boolean::New(b)
#define JS_throw(t, s)                 ThrowException(Exception::t(String::New(s)))
#define JS_error(s)                    ThrowException(Exception::Error(String::New(s)))

// ----------------------------------------------------------------------------
//
// ARGUMENTS CONVERSION
//
// ----------------------------------------------------------------------------

/**
 * This set of macro make it easy to declare a function that can be exported to
 * the JavaScript environment.
 * See the 'posix.cpp' module for examples. */
#define ARGC                           args.Length()
#define ARG_COUNT(c)                   if ( args.Length() != 0 ) {} 
#define ARG_BETWEEN(a,b)               if ( a <= args.Length() <= b ) {} 
#define ARG_int(n,c)                   int n=(int)(args[c]->Int32Value())
#define ARG_str(v,i)                   v8::String::AsciiValue v(args[i]);
#define ARG_utf8(v,i)                  v8::String::Utf8Value  v(args[i])
#define ARG_obj(v,i)                   v8::Local<v8::Object> v=args[i]->ToObject();
#define ARG_obj(v,i)                   v8::Local<v8::Object> v=args[i]->ToObject();
#define ARG_array(name, c) \
		if (!args[(c)]->IsArray()) { \
			//std::ostringstream __k7_e; \
			//__k7_e << "Exception: argument error." << __func__ << " expects array for argument " << c << "."; \
			//return ThrowException(String::New(__k7_e.str().c_str())); \
		} \
		Handle<Array> name = Handle<Array>::Cast(args[(c)])
#define ARG_fn(name, c) \
		Handle<Function> name = Handle<Function>::Cast(args[(c)])



// ----------------------------------------------------------------------------
//
// FUNCTIONS MACROS
//
// ----------------------------------------------------------------------------

#define FUNCTION_DECL(f)               static v8::Handle<v8::Value> f(const v8::Arguments&);
#define FUNCTION(f)                    static v8::Handle<v8::Value> f(const v8::Arguments& args) { v8::HandleScope handlescope;
#define FUNCTION_C(f)                  static v8::Handle<v8::Value> f(const v8::Arguments& args) {
#define END                            }
#define THIS                           args.This()
#define STUB                           return ThrowException(Exception::Error(String::New("Stub - Function not implemented")));
 
// ----------------------------------------------------------------------------
//
// OBJECT MACROS
//
// ----------------------------------------------------------------------------

/**
 * This set of macros allow you to declare a new object type, usually because
 * you want to set internal data in this object. 
 * See the 'posix.cpp' module for examples. */

#define OBJECT(name,fields,...) \
	v8::Handle<Object> name(__VA_ARGS__) { \
		v8::HandleScope handle_scope;\
		v8::Handle<v8::FunctionTemplate>   __class__  = v8::FunctionTemplate::New();\
		v8::Handle<v8::ObjectTemplate>     __object__ = __class__->InstanceTemplate();\
		__object__->SetInternalFieldCount(fields);\
		v8::Handle<v8::Object>               self = __object__->NewInstance();

#define INTERNAL(i,value) \
	self->SetInternalField(i, v8::External::New((void*)value));

#define EXTERNAL(type,name,object,indice) \
	type name = (type) (v8::Local<v8::External>::Cast(object->GetInternalField(0))->Value());

// ----------------------------------------------------------------------------
//
// CLASS MACROS
//
// ----------------------------------------------------------------------------

#define CLASS(name)\
	{ \
		v8::Handle<v8::String>            __class_name__ = v8::String::New(name); \
		v8::Handle<v8::FunctionTemplate>  __class__      = v8::FunctionTemplate::New(); \
		v8::Handle<v8::ObjectTemplate>    __object__     = __class__->InstanceTemplate(); \
		v8::Handle<v8::ObjectTemplate>    self           = __object__; \
		__class__->SetClassName(v8::String::New(name)); 
#define CONSTRUCTOR(c)       __class__->SetCallHandler(c);
#define INTERNAL_FIELDS(i)   __object__->SetInternalFieldCount(i);
#define END_CLASS            __module__->Set(__class_name__,__class__->GetFunction(),v8::PropertyAttribute(v8::ReadOnly|v8::DontDelete));}

// ----------------------------------------------------------------------------
//
// MODULE MACROS
//
// ----------------------------------------------------------------------------

#define MODULE(symbol,moduleName)\
	extern "C" v8::Handle<v8::Object> symbol (v8::Handle<v8::Object>__module__) {\
		v8::Handle<v8::Object> self = __module__;
#define END_MODULE                  return __module__; }

// ----------------------------------------------------------------------------
//
// SCOPE AND BINDING MACROS
//
// ----------------------------------------------------------------------------

#define WITH_CLASS        { v8::Handle<v8::Object> self = __class__;
#define WITH_MODULE       { v8::Handle<v8::Object> self = __module__;
#define WITH_OBJECT       { v8::Handle<v8::Object> self = __object__;

#define SET(s,v)          self->Set(JS_str(s),v);
#define SET_int(s,v)      self->Set(JS_str(s), JS_int(v));
#define SET_str(s,v)      self->Set(JS_str(s), JS_str(v));

#define BIND(s,v)         self->Set(JS_str(s),v8::FunctionTemplate::New(v)->GetFunction());
#define METHOD(s,v)       __object__->Set(JS_str(s),v8::FunctionTemplate::New(v)->GetFunction());
#define CLASS_METHOD(s,v) __class__->Set(JS_str(s),v8::FunctionTemplate::New(v)->GetFunction());

// ----------------------------------------------------------------------------
//
// API MACROS
//
// ----------------------------------------------------------------------------

/**
 * These macros allow to declare a module initialization function and register
 * FUNCTIONs in this module. */
#define LINK_TO(lib)
#define ENVIRONMENT                 void SetupEnvironment (v8::Handle<v8::Object> global,int argc, char** argv, char** env) {
#define IMPORT(function)            extern "C" v8::Handle<v8::Object> function(v8::Handle<v8::Object> module);
#define LOAD(moduleName,function)   function(EnsureModule(global,moduleName));
#define EVAL(source)                ExecuteString(JS_str(source), JS_undefined, false);

#endif
// EOF - vim: ts=4 sw=4 noet
