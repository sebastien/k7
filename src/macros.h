#define  JS_str(s)                      v8::String::New(s)
#define  JS_int(s)                      v8::Integer::New(s)
#define  JS_undefined                   v8::Undefined()
#define  JSOBJ_set(target,slot,value)   target->Set(JS_str(slot),value)  

#define  FUNCTION(f)                    v8::Handle<v8::Value> f(const v8::Arguments& args) { v8::HandleScope handlescope;
#define  ARG_COUNT(c)                   if ( args.Length() != 0 ) {} 
#define  ARG_int(n,c)                   int n=(int)(args[c]->Int32Value())
#define  ARG_str(v,i)                   v8::String::AsciiValue v(args[i])
#define  ARG_obj(v,i)                   Local<Object> v=args[i]->ToObject();
#define  END                            }

#define OBJECT(name,fields,...) \
Handle<Object> name(__VA_ARGS__) { \
	HandleScope handle_scope;\
	Handle<FunctionTemplate>   fun_template = FunctionTemplate::New();\
	Handle<ObjectTemplate>   obj_template = fun_template->InstanceTemplate();\
	obj_template->SetInternalFieldCount(fields);\
	Handle<Object> self = obj_template->NewInstance();\

#define INTERNAL(i,value) \
	self->SetInternalField(i, External::New((void*)value));

#define EXTERNAL(type,name,object,indice) \
	type name = (type) (Local<External>::Cast(object->GetInternalField(0))->Value());

#define  INIT                           v8::Handle<v8::Value> instantiate() {\
    HandleScope handle_scope; \
    Handle<Object> module = Object::New();

#define  END_INIT                       return module; }
#define  BIND(s,v)                      module->Set(JS_str(s),FunctionTemplate::New(v)->GetFunction());

#define  V8_FT(f)                       v8::FunctionTemplate::New(f)
