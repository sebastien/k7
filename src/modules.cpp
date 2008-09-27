// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
// ----------------------------------------------------------------------------
// Creation date     : 27-Sep-2008
// Last modification : 27-Sep-2008
// ----------------------------------------------------------------------------

#include <v8.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "modules.h"

using namespace v8;

v8::Handle<v8::Object>     EnsureModule (
	v8::Handle<v8::Object>   parent,
	const char*              moduleName,
	const char*              fullName
) {

	HandleScope       handle_scope;
	Handle<Object>    module;

	// TODO: Rewrite this in proper C++ style
	// This section simply splits the "full.module.name" so that
	// module_name = "full" and rest = "module.name" 
	// and fullName =// "full.module.name"
	const char*       rest = strpbrk(moduleName, ".");
	char*             module_name;
	if ( fullName == NULL ) {
		fullName = moduleName;
	}
	if ( rest == NULL ) {
		int len = strlen(moduleName);
		module_name = (char*) malloc((len + 1) * sizeof(char));
		strncpy(module_name, moduleName, len + 1); 
	} else {
		int len = rest - moduleName;
		module_name = (char*) malloc((len  + 1) * sizeof(char));
		strncpy(module_name, moduleName, len); 
		module_name[len] = '\0';
		rest ++;
	}

	// This is where we create the actual module object. The module name is
	// bound to the "__module__" attribute.
	Local<v8::String> name = v8::String::New(module_name);
	if (parent->Has(name)) {
		module = parent->Get(name)->ToObject();
		// The module may have already been created by a submodule, so if this
		// is the module we wanted to ensure, we make sure it has the __module__
		// property.
		if (rest == NULL) {
			if (!module->Has(v8::String::New("__module__"))) {
				module->Set (v8::String::New("__module__"), v8::String::New(fullName));
			}
		}
	} else {
		Local<ObjectTemplate> obj_template = FunctionTemplate::New()->InstanceTemplate();
		// We only set the __module__ slot for the module we are loading.
		if (rest == NULL) {
			obj_template->Set(v8::String::New("__module__"), v8::String::New(fullName));
		}
		module = obj_template->NewInstance();
		parent->Set(name,module);
	}
	free(module_name);

	// If the module had prefixes, we make sure that the parent module exists
	if ( rest == NULL ) {
		return module;
	} else {
		return EnsureModule(module, rest, fullName);
	}
}
// EOF - vim: ts=4 sw=4 noet
