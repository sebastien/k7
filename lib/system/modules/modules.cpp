// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
// ----------------------------------------------------------------------------
// Creation date     : 17-Mar-2009
// Last modification : 08-May-2009
// ----------------------------------------------------------------------------

#define MODULE_NAME   "system.modules"
#define MODULE_STATIC  system_modules
#include <k7.h>

#include <stdlib.h>
#include <time.h>


FUNCTION(module_has)
/**
 * Tells if the module with the given name exists in the current sugar
 * installation.
*/
{
	ARG_COUNT(0)
}
END

FUNCTION(module_load)
// Loads the given module into the current interpreter
{
	ARG_COUNT(0)
	STUB
}
END

FUNCTION(module_ensure)
{
	ARG_COUNT(1)
	ARG_str(fullName, 0);
	return k7::module(*fullName);
}
END

FUNCTION(module_locate, PSTR(moduleName))
{
	return k7::locate(*moduleName);
}
END

FUNCTION(module_update)
{
	ARG_COUNT(2)
	ARG_obj(module, 0);
	ARG_obj(update, 1);
	OBJECT_COPY_SLOTS(update, module);
	return module;
}
END

MODULE
{
	#include "modules.js.h"
	BIND("has",     module_has);
	BIND("load",    module_load);
	BIND("locate",  module_locate);
	BIND("ensure",  module_ensure);
	BIND("update",  module_update);
	EXEC(MODULES_JS)
	// update
	// create
	// ensure
}
END_MODULE

// EOF - vim: ts=4 sw=4 noet
