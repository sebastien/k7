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

MODULE(data_formats_json,"data.formats.json")
	#include "json.h"
	EVAL(JSON_JS)
END_MODULE

// EOF - vim: ts=4 sw=4 noet
