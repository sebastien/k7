// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
// ----------------------------------------------------------------------------
// Creation date     : 27-Sep-2008
// Last modification : 28-Sep-2008
// ----------------------------------------------------------------------------

#include "modules.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#define SHARED_OJECT_SUFFIX         ".so"
#define JS_SUFFIX                   ".js"
#define MODULE_SUFFIX_MAXLEN        4
using namespace v8;

/**
 * Finds a path that corresponds to the given module name.
 * This functions returns NULL when no module was found, or returns a new string
 * containing the path to the module. Don't forget to 'free' the string
 * afterwards.
*/
char* FindModule ( const char* moduleName) {
	int   length      = strlen(moduleName);
	char* module_path = (char*)malloc((length + MODULE_SUFFIX_MAXLEN + 1)*sizeof(char));

	strcpy(module_path, moduleName);

	// We replace the '.' in the modulename by directory separators "/"
	for( char* offset=module_path ; offset!=NULL ; offset=strpbrk(offset,".") ) {
		if (offset!=module_path) { *(offset) = '/'; }
	}

	// We look for a '.so' file
	strcpy((module_path+length), SHARED_OJECT_SUFFIX);
	FILE* fd = fopen(module_path,"r");
	if (fd!=NULL) {
		fclose(fd);
		return module_path;
	}

	// Or for a '.js' file
	strcpy((module_path+length), JS_SUFFIX);
	fd = fopen(module_path,"r");
	if (fd!=NULL) {
		fclose(fd);
		return module_path;
	} else {
		free(module_path);
		return (char*)NULL;
	}
}

/**
 * Loads a module into the global environment, if the module is already loaded,
 * it will be returned, otherwise it will be loaded and initialized.
*/
void LoadModule(
	v8::Handle<v8::Object>  global,
	const char*             moduleName,
	const char*             modulePath
) {
	char* module_path;
	if (modulePath==NULL) {
		module_path = FindModule(moduleName);
	} else {
		module_path = (char*)malloc((strlen(modulePath)+1)*sizeof(char));
		strcpy(module_path,modulePath);
	}
	//printf("LOADING MODULE:%s\n", modulePath);
	// TODO: Do the distinction between a .js and a .so file
	void* handle = dlopen(module_path, RTLD_LAZY);
	const char *dlsym_error;
	if (handle) {
		module_init_t init = (module_init_t) dlsym(handle,"initialize");
		dlsym_error = dlerror();
		if (dlsym_error) {
			//printf("ERROR: %s\n", dlsym_error);
			dlclose(handle);
		} else {
			//printf("Loading module init\n", dlsym_error);
			init(global);
		}
	}
	if ( module_path!=NULL ) { free(module_path); }
}

// EOF - vim: ts=4 sw=4 noet
