// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
// ----------------------------------------------------------------------------
// Creation date     : 03-Mar-2009
// Last modification : 03-Mar-2009
// ----------------------------------------------------------------------------

#include "macros.h"

#define MODULE_NAME   "core.extend"
#define MODULE_STATIC  core_extend

MODULE
{
	#include "extend.js.h"
	EXEC(EXTEND_JS)
}
END_MODULE

// EOF - vim: ts=4 sw=4 noet
