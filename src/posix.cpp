#define MODULE posix

//OBJECT(posix_FILE, FILE* file)
//	INTERNAL_FIELD(file)
//END

Handle<Object> posix_FILE(FILE* file) {

	HandleScope handle_scope;
	Handle<ObjectTemplate>   obj_template = ObjectTemplate::New();
	obj_template->SetInternalFieldCount(1);
	Handle<Object> self = obj_template->NewInstance();

	//self->SetInternalField(0, External::New((void*)file));

	return self;
}

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
