#define MODULE posix

//OBJECT(posix_FILE, FILE* file)
//	INTERNAL_FIELD(file)
//END

#define OBJECT(name,fields,...) \
Handle<Object> name(__VA_ARGS__) { \
	HandleScope handle_scope;\
	Handle<FunctionTemplate>   fun_template = FunctionTemplate::New();\
	Handle<ObjectTemplate>   obj_template = fun_template->InstanceTemplate();\
	obj_template->SetInternalFieldCount(fields);\
	Handle<Object> self = obj_template->NewInstance();\

#define INTERNAL(i,value) \
	self->SetInternalField(i, External::New((void*)value));

OBJECT(posix_FILE,1,FILE* file)
	INTERNAL(0,file)
	return self;
END

FUNCTION(posix_time)
	EXPECT_ARGS(==0)
	return V8_int(time(NULL));
END

FUNCTION(posix_fopen)
	EXPECT_ARGS(==2)
	ARG_str(path,0);
	ARG_str(mode,1);
	FILE* fd = fopen(*path,*mode);
	return posix_FILE(fd);
END

START_INIT
	BIND("time",  posix_time);
	BIND("fopen", posix_fopen);
END_INIT
