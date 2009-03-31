/**
 * An interface to the javascript engine.
 * In k7, for now, this is v8.  However, in the future,
 * a different engine might be a better choice.
 * So, this module is not named v8, since
 * k7 is intended to be as much a standard as a
 * useful reference implementation. — isaacs
 **/

#include "macros.h"
#include <stdlib.h>
#include <time.h>
#include <string>

using namespace v8;

FUNCTION(getState)
{
	STUB
}
END

FUNCTION(setState)
{
	STUB
}
END

FUNCTION(compileScript)
{
	STUB
}
END

FUNCTION(runScript)
{
	STUB
}
END

FUNCTION(defineProperty)
{
	STUB
}
END

FUNCTION(getPropertyDefinition)
{
	STUB
}
END

FUNCTION(sealObject)
{
	STUB
}
END

FUNCTION(freezeObject)
{
	STUB
}
END
	

MODULE(system_engine, "system.engine")
{
	BIND("getState", getState);
	BIND("setState", setState);
	BIND("compileScript", compileScript);
	BIND("runScript", runScript);
	BIND("defineProperty", defineProperty);
	BIND("getPropertyDefinition", getPropertyDefinition);
}
END_MODULE
