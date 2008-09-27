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
	ARG_COUNT(0)
	return JS_int(time(NULL));
END

#define EXTERNAL(type,name,object,indice) \
	type name = (type) (Local<External>::Cast(object->GetInternalField(0))->Value());

FUNCTION(posix_fwrite)
	ARG_str(data,0);
	ARG_int(size,1);
	ARG_int(nmemb,2);
	ARG_obj(fileObj,3);
	EXTERNAL(FILE*,file,fileObj,0);
	return JS_int(fwrite(*data,size,nmemb,file));
END

FUNCTION(posix_fopen)
	ARG_COUNT(2)
	ARG_str(path,0);
	ARG_str(mode,1);
	FILE* fd = fopen(*path,*mode);
	return posix_FILE(fd);
END

FUNCTION(posix_fclose)
	ARG_COUNT(1);
	ARG_obj(fileObj,0);
	EXTERNAL(FILE*,file,fileObj,0);
	return JS_int(fclose(file));
END

START_INIT
	BIND("time",   posix_time);
	BIND("fopen",  posix_fopen);
	BIND("fwrite", posix_fwrite);
	BIND("fclose", posix_fclose);
END_INIT
