#define MODULE posix

FUNCTION(posix_time)
EXPECT_ARG_COUNT(==0)
	return v8::Integer::New(time(NULL));
END

INIT
	HandleScope handle_scope;
	DECLARE("time", posix_time)
	return module;
END
