// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
// Author            : Isaac Schulter                            <i@foohack.com>
// ----------------------------------------------------------------------------
// Creation date     : 27-Sep-2008
// Last modification : 09-Sep-2009
// ----------------------------------------------------------------------------

#include <v8.h>
#include <k7.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <time.h>
#include <sys/stat.h>

#include <dlfcn.h>

// NOTE: You may be surprised that this file does not contain much code. As
// K7 aims at implementing as much as possible in JavaScript, most of the
// K7 code is implemented in its modules, which can be foud in the 'lib'
// directory. The core K7 code is merely a set of simple functions and a
// bootstrapping logic to setup the module system and the shell environment.

// ----------------------------------------------------------------------------
//
// K7 ENVIRONMENT
//
// ----------------------------------------------------------------------------

// Imports the symbols from standard libraries that can be found in the
// "lib" directory of the source tree
#ifdef STATIC
	IMPORT(system_modules);
	IMPORT(system_shell);
	IMPORT(system_posix);
	IMPORT(system_engine);
	IMPORT(core_extend);
	IMPORT(data_formats_json);
	IMPORT(net_http_server_shttpd);
	#ifdef WITH_FCGI
	IMPORT(net_http_server_fcgi);
	#endif
	#ifdef WITH_CURL
	IMPORT(net_http_client_curl);
	#endif
#endif

#ifdef WITH_LIBTASK
IMPORT(core_concurrency_libtask);
#endif

#ifdef WITH_LIBNODE
IMPORT(core_concurrency_libnode);
#endif

/**
 * Sets up the K7 environment, loading the module system and the shell.
 *
*/
void k7::setup (v8::Handle<v8::Object> global,int argc, char** argv, char** env) {

	// We create the environment object
	Handle<Object> js_env = JS_obj();
	if (env != NULL) {
		for (int i = 0; env[i]; i ++) {
			int j;
			for (j = 0; env[i][j] && env[i][j] != '='; j ++);
			env[i][j] = '\0';
			OBJECT_SET(js_env, env[i], JS_str(env[i]+j+1));
		}
	}
	Handle<Array> js_argv = Array::New(argc);
	for (int i = 0; i < argc; i ++) {
		js_argv->Set(JS_int(i), JS_str(argv[i]));
	}
	OBJECT_SET(js_env, "argc", JS_int(argc));
	OBJECT_SET(js_env, "argv", js_argv);
	OBJECT_SET(k7::module("system"), "ENV",    js_env);
	OBJECT_SET(k7::module("system"), "GLOBAL", JS_GLOBAL);

	#ifndef STATIC
		// In dynamic mode, we only need modules and shell modules, the
		// rest will be loaded on demand
		k7::dynload("build/plugins/system/modules/modules.so", global);
		k7::dynload("build/plugins/system/shell/shell.so",     global);
	#else
		LOAD("system.modules",      system_modules);
		LOAD("system.shell",        system_shell);

		// NOTE: This is no good as it slows down the startup time,
		// especially when there is pure JavaScript that requires parsing.

		LOAD("system.posix",           system_posix);
		LOAD("system.engine",          system_engine);
		LOAD("core.extend",            core_extend);
		LOAD("data.formats.json",      data_formats_json);
		LOAD("net.http.server.shttpd", net_http_server_shttpd);
		#ifdef WITH_FCGI
		LOAD("net.http.server.fcgi",   net_http_server_fcgi);
		#endif
		#ifdef WITH_CURL
		LOAD("net.http.client.curl",   net_http_client_curl);
		#endif
	#endif
	#ifdef WITH_LIBNODE
	LOAD("core.concurrency.libnode",   core_concurrency_libnode);
	#endif
	#ifdef WITH_LIBTASK
	LOAD("core.concurrency.libtask",   core_concurrency_libtask);
	#endif
}

// ----------------------------------------------------------------------------
//
// MAIN SHELL FUNCTIONS
//
// ----------------------------------------------------------------------------

/**
 *  Reports the given exception on stderr
*/
void k7::trace (TryCatch &try_catch) {
	Handle<Message> message = try_catch.Message();
	if (message.IsEmpty()) {
		fprintf(stderr, "---\n[!] K7 Error: (no message)\n");
		return;
	}
	Handle<Value>  error = try_catch.Exception();
	Handle<String> stack;

	if (error->IsObject()) {
		Handle<Object> obj = Handle<Object>::Cast(error);
		Handle<Value> raw_stack = obj->Get(String::New("stack"));
		if (raw_stack->IsString()) stack = Handle<String>::Cast(raw_stack);
	}
	if (stack.IsEmpty()) {
		String::Utf8Value exception(error);

		// Print (filename):(line number): (message).
		String::Utf8Value filename(message->GetScriptResourceName());
		const char* filename_string = *filename;
		int linenum = message->GetLineNumber();
		fprintf(stderr, "---\n[!] K7 Error in %s:%i: %s\n", filename_string, linenum, *exception);

		// Print line of source code.
		String::Utf8Value sourceline(message->GetSourceLine());
		const char* sourceline_string = *sourceline;
		fprintf(stderr, "    %s\n    ", sourceline_string);
		// Print wavy underline (GetUnderline is deprecated).
		int start = message->GetStartColumn();
		for (int i = 0; i < start; i++) {
		  fprintf(stderr, " ");
		}
		int end = message->GetEndColumn();
		for (int i = start; i < end; i++) {
		  fprintf(stderr, "^");
		}
		fprintf(stderr, "\n");

		message->PrintCurrentStackTrace(stderr);
	} else {
		String::Utf8Value trace(stack);
		fprintf(stderr, "---\n[!] K7 error %s\n", *trace);
	}
}

/**
 * Executes the given C string, optionaly coming from the given file.
*/
bool k7::execute (const char* source)                           { return k7::execute(JS_str(source), JS_str("(shell)")); }
bool k7::execute (const char* source, const char* fromFileName) { return k7::execute(JS_str(source), JS_str("fromFileName")); }
bool k7::execute (Handle<String> source)                        { return k7::execute(source,         JS_str("(shell)")); }
bool k7::execute (Handle<String> source, Handle<Value> fromFileName) {
	if (source->Length() == 0) return true;
	HandleScope handle_scope;
	Handle<Value> exception;
	
	// FIXME: We disabled this, as we registered a V8 message listener
	TryCatch try_catch;
	try_catch.SetCaptureMessage(true);
	try_catch.SetVerbose(true);

	String::Utf8Value utf8_value(source);

	Handle<Script> script = Script::Compile(source, fromFileName);
	if (script.IsEmpty()) {
		// FIXME: We disabled this, as we registered a V8 message listener
		//exception = try_catch.Exception();
		k7::trace(try_catch);
		return false;
	}
	Handle<Value> result = script->Run();
	if (result.IsEmpty()) {
		// FIXME: We disabled this, as we registered a V8 message listener
		//exception = try_catch.Exception();
		k7::trace(try_catch);
		return false;
	}
	if ( !exception.IsEmpty() ) {
		ThrowException(exception);
		return false;
	} else {
		return true;
	}
	return true;
}

/**
 * Like execute, but returns an object or undefined if a problem happened
*/
Handle<Value> k7::eval (const char* source)                           { return k7::eval(JS_str(source), JS_str("(shell)")); }
Handle<Value> k7::eval (const char* source, const char* fromFileName) { return k7::eval(JS_str(source), JS_str(fromFileName)); }
Handle<Value> k7::eval (Handle<String> source)                        { return k7::eval(source,         JS_str("(shell)")); }
Handle<Value> k7::eval (Handle<String> source, Handle<Value> fromFileName) {
	if (source->Length() == 0) return JS_undefined;
	HandleScope handle_scope;
	// FIXME: We disabled this, as we registered a V8 message listener
	TryCatch try_catch; 
	String::Utf8Value utf8_value(source);

	Handle<Script> script = Script::Compile(source, fromFileName);
	if (script.IsEmpty()) {
		// FIXME: We disabled this, as we registered a V8 message listener
		k7::trace(try_catch);
		return JS_undefined;
	}
	Handle<Value> result = script->Run();
	if (result.IsEmpty()) {
		// FIXME: We disabled this, as we registered a V8 message listener
		k7::trace(try_catch);
		return JS_undefined;
	}
	return result;
}

Handle<Object> k7::module(const char* fullName) {
	return k7::module(JS_GLOBAL, fullName, NULL);
}

Handle<Value> k7::locate  (const char* moduleName) {
	return k7::locate(moduleName, ".");
}

Handle<Value> k7::locate  (const char* moduleName, const char* inPath) {
	// Basically takes the given inPath, use it as a prefix for the given
	// moduleName, replacing the '.' by separators.
	struct stat stat_info;
	int  path_len = strlen(inPath);
	int  name_len = strlen(moduleName);
	char* module_path    = (char*)malloc(name_len + 1 + path_len + 3 + 1);
	char* module_name    = module_path + path_len;
	strcpy(module_path, inPath);
	// We add an extra OS_SEPARATOR if not there
	if (module_path[path_len - 1] != OS_SEPARATOR) {
		module_path[path_len] = OS_SEPARATOR;
		module_name += 1;
	}
	strcpy(module_name, moduleName);
	// We replace os separators by '.'
	for (int i=0 ; i < name_len ; i++) {
		if (module_name[i] == OS_SEPARATOR) {
			module_name[i] = '.';
		}
	}
	module_name[name_len + 0] = '.';
	module_name[name_len + 1] = 's';
	module_name[name_len + 2] = 'o';
	module_name[name_len + 3] = '\0';
	// If we can't access the .so file, we'll try the .js version
	if ( stat(module_path, &stat_info) == -1 ) {
		module_name[name_len + 1] = 'j';
		module_name[name_len + 2] = 's';
		// If we still can't use it
		if ( stat(module_path, &stat_info) == -1 ) {
			free(module_path);
			return  JS_undefined;
		} else {
			Handle<String> res = JS_str(module_path);
			free(module_path);
			return res;
		}
	} else {
		Handle<String> res = JS_str(module_path);
		free(module_path);
		return res;
	}
}

FUNCTION(module_toString) {
	return OBJECT_GET(THIS, "__name__");
} END

Handle<Object> k7::module(Handle<Object>  parent, const char* moduleName, const char* fullName) {

	// FIXME: I had to disable the HandleScope, as it seems like the created
	// modules are garbage collected, and this causes problems.
	// HandleScope       handle_scope;
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
			if (!module->Has(v8::String::New("__name__"))) {
				module->Set (v8::String::New("__name__"), v8::String::New(fullName));
			}
		}
	} else {
		// We only set the __module__ slot for the module we are loading.
		module = Object::New();
		if (rest == NULL) {
			module->Set(v8::String::New("__name__"), v8::String::New(fullName));
		}
		Handle<Function> to_string   = v8::FunctionTemplate::New(module_toString)->GetFunction();
		Handle<String>   to_string_n = JS_str("toString");
		to_string->SetName(to_string_n);
		module->Set(to_string_n,to_string);
		parent->Set(name,module);
	}
	free(module_name);

	// If the module had prefixes, we make sure that the parent module exists
	if ( rest == NULL ) {
		return module;
	} else {
		return k7::module(module, rest, fullName);
	}
}

Handle<Value> k7::dynload (const char* modulePath, Handle<Object> parent) {
	void* k7_plugin;
	Handle<Object> (*init_function)(Handle<Object> global, K7_MODULE_CREATOR_T) = NULL;
	k7_plugin = dlopen(modulePath, RTLD_NOW);
	if (k7_plugin == NULL) {
		fprintf(stderr, "[!] K7: Cannot load plugin '%s':%s\n", modulePath, dlerror());
	} else {
		init_function = (Handle<Object> (*)(Handle<Object> g, K7_MODULE_CREATOR_T)) dlsym(k7_plugin, "k7_module_init");
		if (init_function == NULL) {
			fprintf(stderr, "[!] K7: Init function not found in plugin '%s':%s\n", modulePath, dlerror());
		} else {
			return init_function(parent, k7::module);
		}
	}
	return JS_undefined;
}

Handle<Value> k7::read(const char* path) {
	FILE* file = fopen(path, "rb");
	if (file == NULL) {
		return JS_null;
	}

	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	rewind(file);

	char* chars = new char[size + 1];
	chars[size] = '\0';
	for (int i = 0; i < size;) {
		int read = fread(&chars[i], 1, size - i, file);
		i += read;
	}
	fclose(file);
	v8::Handle<v8::String> result = v8::String::New(chars, size);
	delete[] chars;
	return result;
}

/**
 * This is the main function that sets up the K7 environment
*/
int k7::main (int argc, char **argv, char **env) {

	HandleScope handle_scope;
	//V8::AddMessageListener(k7::onMessage);

	// Create a template for the global object.
	Handle<ObjectTemplate> global_template = ObjectTemplate::New();

	// Create a new execution environment containing the built-in
	// functions
	Handle<Context> context = Context::New(NULL, global_template);
	Handle<Object>  global  = context->Global();

	// Enter the newly created execution environment.
	Context::Scope context_scope(context);

	k7::setup(context->Global(), argc, argv, env);

	// TODO: If we use libevent, we should schedule the execution of this on
	// to the libevent loop, otherwise we'll have code running in different threads,
	// which is something we want to avoid.
	EXEC("system.shell.command();")

	//V8::RemoveMessageListeners(k7::onMessage);

	return 0;
}

// ----------------------------------------------------------------------------
//
// THE C MAIN
//
// ----------------------------------------------------------------------------

#ifndef __K7_LIBRARY_ONLY__
extern "C" {
	#ifdef WITH_LIBTASK
		void taskmain (int argc, char **argv) {
			k7::main(argc, argv, NULL);
		}
	#else
		int main (int argc, char **argv, char **env) {
			return k7::main(argc, argv, env);
		}
	#endif
}
#endif

// EOF - vim: ts=4 sw=4 noet
