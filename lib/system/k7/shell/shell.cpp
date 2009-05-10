// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
// ----------------------------------------------------------------------------
// Creation date     : 08-May-2008
// Last modification : 08-May-2009
// ----------------------------------------------------------------------------

#include "k7.h"

#define MODULE_NAME   "system.k7.shell"
#define MODULE_STATIC  system_k7_shell

// ----------------------------------------------------------------------------
//
// BASIC SHELL FUNCTIONS
//
// ----------------------------------------------------------------------------

FUNCTION(shell_print)
{
	bool first = true;
	for (int i = 0; i < args.Length(); i++) {
		if (first) {
			first = false;
		} else {
			printf(" ");
		}
		ARG_str(str,i);
		printf("%s", *str);
	}
	return Boolean::New(true);
}
END

FUNCTION(shell_load)
{
	ARG_COUNT(1);
	ARG_str(file, 0)
	return k7::load(*file);
}
END

FUNCTION(shell_run)
{
	ARG_COUNT(1);
	ARG_str(file, 0)
	Handle<Value> source    = k7::load(*file);
	if (source->IsString()) {
		Local<String> previous = OBJECT_GET(JS_GLOBAL,"__file__")->ToString();
		OBJECT_SET(JS_GLOBAL,"__file__", JS_str(*file));
		Handle<Value> result = k7::eval(source->ToString());
		OBJECT_SET(JS_GLOBAL,"__file__", previous);
		return result;
	} else {
		ERROR("shell.run: File not found");
	}
	return JS_undefined;
}
END

// ----------------------------------------------------------------------------
//
// THE MODULE
//
// ----------------------------------------------------------------------------

MODULE
{
	//BIND("ARGV",    k7_module_load);
	//BIND("VERSION", k7_module_load);
	//BIND("Global",  k7_module_has);
	BIND("print",   shell_print);
	BIND("run",     shell_run);
	BIND("load",    shell_load);
	#include "shell.js.h"
	EXEC(SHELL_JS)
}
END_MODULE

// EOF - vim: ts=4 sw=4 noet
