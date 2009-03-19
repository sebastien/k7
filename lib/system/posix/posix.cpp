// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
// ----------------------------------------------------------------------------
// Creation date     : 27-Sep-2008
// Last modification : 19-Mar-2009
// ----------------------------------------------------------------------------

#include "macros.h"
#include <stdlib.h>
#include <time.h>
#include <string>

// TODO: Add proper error handling
// #define MODULE "system.posix"
using namespace v8;

OBJECT(posix_FILE,1,FILE* file)
	INTERNAL(0,file)
	return self;
END

FUNCTION(posix_time)
	ARG_COUNT(0)
	return JS_int(time(NULL));
END

FUNCTION(posix_fopen)
	ARG_COUNT(2)
	ARG_utf8(path,0);
	ARG_utf8(mode,1);
	FILE* fd = fopen(*path,*mode);
	return posix_FILE(fd);
END

FUNCTION(posix_fclose)
	ARG_COUNT(1);
	ARG_obj(fileObj,0);
	EXTERNAL(FILE*,file,fileObj,0);
	return JS_int(fclose(file));
END

FUNCTION(posix_popen)
	ARG_COUNT(2)
	ARG_utf8(path,0);
	ARG_utf8(type,1);
	FILE* fd = popen(*path,*type);
	return posix_FILE(fd);
END

FUNCTION(posix_pclose)
	ARG_COUNT(1);
	ARG_obj(fileObj,0);
	EXTERNAL(FILE*,file,fileObj,0);
	return JS_int(pclose(file));
END

FUNCTION(posix_system)
	ARG_COUNT(1);
	ARG_utf8(command,0);
	return JS_int(system(*command));
END

FUNCTION(posix_fwrite)
	ARG_utf8(data,0);
	ARG_int(size,1);
	ARG_int(nmemb,2);
	ARG_obj(fileObj,3);
	EXTERNAL(FILE*,file,fileObj,0);
	return JS_int(fwrite(*data,size,nmemb,file));
END

FUNCTION(posix_fread)
	ARG_COUNT(3);
	ARG_int(size,0);
	ARG_int(nmemb,1);
	ARG_obj(fileObj,2);
	EXTERNAL(FILE*,file,fileObj,0);
	if (size < 0) {
		return v8::ThrowException(JS_str("Exception: invalid bufsize"));
	}
	char *buf   = new char[size*nmemb + 1024];
	size_t read = fread(buf, size, nmemb, file);
	v8::Handle<v8::String> strbuf = JS_str2(buf, read);
	delete [] buf;
	return strbuf;
END

FUNCTION(posix_feof)
{
	ARG_obj(fileObj, 1);
	EXTERNAL(FILE*,file,fileObj,0);
	return Boolean::New(feof(file));
}
END
	
FUNCTION(posix_readfile)
	STUB
END

FUNCTION(posix_writefile)
{
	ARG_COUNT(2);
	ARG_utf8(name, 0);
	ARG_utf8(data, 1);
	
	if (*name == NULL) {
		return v8::ThrowException(v8::String::New("Invalid filename"));
	}

	FILE* file = fopen(*name, "wb");
	if (file == NULL) {
		return v8::ThrowException(v8::String::New("Could not open file for writing."));
	}
	int result = fwrite(*data,strlen(*data),1,file);
	fclose(file);
	
	return JS_int(result);
	
}
END

MODULE(system_posix,"system.posix")
	// FIXME: When I set the module 'time' slot to a string, accessing the slot
	// from JavaScript works, but when I BIND it to the posix_time function, the
	// JavaScript returns undefined. Even worse, the next BIND has no effect.
	//module->Set(v8::String::New("time"), JS_str("system.posix.time is a string"));
	BIND("time",      posix_time);
	BIND("fopen",     posix_fopen);
	BIND("fwrite",    posix_fwrite);
	BIND("fread",     posix_fread);
	BIND("fclose",    posix_fclose);
	BIND("writefile", posix_writefile);
	BIND("readfile",  posix_readfile);
	BIND("feof",      posix_feof);
	BIND("popen",     posix_popen);
	BIND("pclose",    posix_pclose);
	BIND("system",    posix_system);
END_MODULE

// EOF - vim: ts=4 sw=4 noet
