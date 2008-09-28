// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
// ----------------------------------------------------------------------------
// Creation date     : 27-Sep-2008
// Last modification : 27-Sep-2008
// ----------------------------------------------------------------------------

#ifndef __K7_MACROS__
#define __K7_MACROS__

#include <v8.h>
#include <k7.h>

/**
 * These macros are just shorthand to creat V8/JavaScript values that can be
 * passed to the V8 API or returned to the JavaScript environment */
#define  JS_str(s)                      v8::String::New(s)
#define  JS_int(s)                      v8::Integer::New(s)
#define  JS_undefined                   v8::Undefined()

#define  JSOBJ_set(target,slot,value)   target->Set(JS_str(slot),value)  
#define  V8_FT(f)                       v8::FunctionTemplate::New(f)

/**
 * This set of macro make it easy to declare a function that can be exported to
 * the JavaScript environment.
 * See the 'posix.cpp' module for examples. */
#define  FUNCTION(f)                    v8::Handle<v8::Value> f(const v8::Arguments& args) { v8::HandleScope handlescope;
#define  ARG_COUNT(c)                   if ( args.Length() != 0 ) {} 
#define  ARG_int(n,c)                   int n=(int)(args[c]->Int32Value())
#define  ARG_str(v,i)                   v8::String::AsciiValue v(args[i])
#define  ARG_obj(v,i)                   v8::Local<v8::Object> v=args[i]->ToObject();
#define  END                            }

/**
 * This set of macros allow you to declare a new object type, usually because
 * you want to set internal data in this object. 
 * See the 'posix.cpp' module for examples. */
#define OBJECT(name,fields,...) \
v8::Handle<Object> name(__VA_ARGS__) { \
	v8::HandleScope handle_scope;\
	v8::Handle<FunctionTemplate>   fun_template = v8::FunctionTemplate::New();\
	v8::Handle<ObjectTemplate>   obj_template = fun_template->InstanceTemplate();\
	obj_template->SetInternalFieldCount(fields);\
	v8::Handle<v8::Object> self = obj_template->NewInstance();
#define INTERNAL(i,value) \
	self->SetInternalField(i, v8::External::New((void*)value));
#define EXTERNAL(type,name,object,indice) \
	type name = (type) (v8::Local<v8::External>::Cast(object->GetInternalField(0))->Value());

/**
 * These macros allow to declare a module initialization function and register
 * FUNCTIONs in this module. */
#define  INIT(name,moduleName) \
    extern "C" v8::Handle<v8::Value> name(v8::Handle<Object> global) {\
    HandleScope    handle_scope; \
    Handle<Object> module = EnsureModule(global,moduleName);

#define  BIND(s,v)       module->Set(JS_str(s),v8::FunctionTemplate::New(v)->GetFunction());

#define ENVIRONMENT      void SetupEnvironment (v8::Handle<v8::Object> global) { \

#define IMPORT(function) extern "C" v8::Handle<v8::Value> function(v8::Handle<Object> global);
#define LOAD(function)   function(global);

#endif
// EOF - vim: ts=4 sw=4 noet
