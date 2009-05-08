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

#define MODULE_NAME   "data.formats.json"
#define MODULE_STATIC  data_formats_json

MODULE
{
	#include "json.js.h"
	EXEC(JSON_JS)
}
END_MODULE

// EOF - vim: ts=4 sw=4 noet
