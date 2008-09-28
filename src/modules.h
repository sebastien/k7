// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
// ----------------------------------------------------------------------------
// Creation date     : 27-Sep-2008
// Last modification : 27-Sep-2008
// ----------------------------------------------------------------------------

#ifndef __K7_MODULE__
#define __K7_MODULE__

#include <v8.h>

typedef v8::Handle<v8::Value> (*module_init_t)(v8::Handle<v8::Object> global);

/**
 * Ensures that the module with the given 'moduleName' exists (which can be
 * dot-separated like 'system.posix' if it is a submodule). The parent modules
 * will be created on the fly if they exist.
 *
 * If the given module does not exist, it will be created, otherwise it will be
 * returned. The module will have a "__module__" property with the fully
 * qualified module name.
 *
 * If the module is a submodule and its parents were not created, they will be
 * created but won't have the "__module__" property (so that we can tell wether
 * they were explicitely loaded or created by a submodule)
*/
v8::Handle<v8::Object>     EnsureModule (
	v8::Handle<v8::Object>   parent,
	const char*              moduleName,
	const char*              fullName=NULL
);
#endif
// EOF - vim: ts=4 sw=4 noet
