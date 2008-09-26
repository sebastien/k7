#define MODULE   "system.posix"
#define REVISION "1.0"

FUNCTION(Posix_fopen)
	v8::Integer::Value fd(args[0]);
	return js_undefined;
END

REGISTRY
	DECLARE(fopen,  Posix_fopen)
	DECLARE(fclose, Posix_fclose)
	DECLARE(fread,  Posix_fread)
	DECLARE(fwrite, Posix_fwrite)
	DECLARE(fwseek, Posix_fwrite)
END

// EOF
