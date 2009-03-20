// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
// ----------------------------------------------------------------------------
// Creation date     : 17-Mar-2009
// Last modification : 17-Mar-2009
// ----------------------------------------------------------------------------

#include <stdlib.h>
#include <time.h>
#include "macros.h"
#include "modules.h"

FUNCTION(k7_module_resolve)
/**
 * Returns a dict {type:..,path:...} representing the module type
 * ('native' or 'source') and its path on the file system. If the module
 * was not found, then 'undefined' is returned.
*/
	ARG_COUNT(0)
END

FUNCTION(k7_module_has)
/**
 * Tells if the module with the given name exists in the current sugar
 * installation.
*/
	ARG_COUNT(0)
END

FUNCTION(k7_module_load)
/**
 * Loads the given module into the current interpreter
*/
	ARG_COUNT(0)
END

MODULE(system_k7_modules,"system.k7.modules")
	#include "modules.h"
	EVAL(MODULES_JS)
	BIND("has",     k7_module_resolve);
	BIND("resolve", k7_module_has);
	BIND("load",    k7_module_load);
END_MODULE

// EOF - vim: ts=4 sw=4 noet
