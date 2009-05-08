// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
// Author            : Isaac Schulter                            <i@foohack.com>
// ----------------------------------------------------------------------------
// Creation date     : 27-Sep-2008
// Last modification : 08-May-2009
// ----------------------------------------------------------------------------

#include <v8.h>
#include <k7.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <time.h>

// NOTE: You may be surprised that this file does not contain much code. As
// K7 aims at implementing as much as possible in JavaScript, most of the
// K7 code is implemented in its modules, which can be foud in the 'lib'
// directory. The core K7 code is merely a set of macros and a simple
// bootstrapping logic to setup the module system and the shell environment.

// ----------------------------------------------------------------------------
//
// K7 ENVIRONMENT
//
// ----------------------------------------------------------------------------

// Imports the symbols from standard libraries that can be found in the
// "lib" directory of the source tree
IMPORT(system_k7_modules);
IMPORT(system_k7_shell);
IMPORT(system_posix);
IMPORT(system_engine);
IMPORT(data_formats_json);
IMPORT(net_http_server_shttpd);
#ifdef WITH_FCGI
IMPORT(net_http_server_fcgi);
#endif
#ifdef WITH_CURL
IMPORT(net_http_client_curl);
#endif

/**
 * Sets up the K7 environment, loading the module system and the shell.
 *
*/
void k7::setup (v8::Handle<v8::Object> global,int argc, char** argv, char** env) {

	// We create the environment object
	Handle<Object> js_env = JS_obj();
	for (int i = 0; env[i]; i ++) {
		int j;
		for (j = 0; env[i][j] && env[i][j] != '='; j ++);
		env[i][j] = '\0';
		OBJECT_SET(js_env, env[i], JS_str(env[i]+j+1));
	}
	Handle<Array> js_argv = Array::New(argc);
	for (int i = 0; i < argc; i ++) {
		js_argv->Set(JS_int(i), JS_str(argv[i]));
	}
	OBJECT_SET(js_env, "argc", JS_int(argc));
	OBJECT_SET(js_env, "argv", js_argv);
	OBJECT_SET(k7::module("system.k7"), "ENV", js_env);

	LOAD("system.k7.modules",      system_k7_modules);
	LOAD("system.k7.shell",        system_k7_shell);
	/*
	LOAD("system.posix",           system_posix);
	LOAD("system.engine",          system_engine);
	LOAD("data.formats.json",      data_formats_json);
	LOAD("net.http.server.shttpd", net_http_server_shttpd);
#ifdef WITH_FCGI
	LOAD("net.http.server.fcgi",   net_http_server_fcgi);
#endif
#ifdef WITH_CURL
	LOAD("net.http.client.curl",   net_http_client_curl);
#endif
*/
}

// ----------------------------------------------------------------------------
//
// MAIN SHELL FUNCTIONS
//
// ----------------------------------------------------------------------------

/**
 *  Reports the given exception on stderr
*/
void k7::trace (const TryCatch* try_catch) {
	HandleScope handle_scope;
	String::Utf8Value exception(try_catch->Exception());
	Handle<Message> message = try_catch->Message();
	if (message.IsEmpty()) {
		// V8 didn't provide any extra information about this error; just
		// print the exception.
		fprintf(stderr, "%s\n", *exception);
	} else {
		// Print (filename):(line number): (message).
		String::Utf8Value filename(message->GetScriptResourceName());
		int linenum = message->GetLineNumber();
		fprintf(stderr, "%s:%i: %s\n", *filename, linenum, *exception);
		// Print line of source code.
		String::Utf8Value sourceline(message->GetSourceLine());
		fprintf(stderr, "%s\n", *sourceline);
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
	TryCatch try_catch;

	String::Utf8Value utf8_value(source);

	Handle<Script> script = Script::Compile(source, fromFileName);
	if (script.IsEmpty()) {
		// Print errors that happened during compilation.
		k7::trace(&try_catch);
		return false;
	}
	Handle<Value> result = script->Run();
	if (result.IsEmpty()) {
		// Print errors that happened during execution.
		k7::trace(&try_catch);
		return false;
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
	TryCatch try_catch; 
	String::Utf8Value utf8_value(source);

	Handle<Script> script = Script::Compile(source, fromFileName);
	if (script.IsEmpty()) {
		// Print errors that happened during compilation.
		k7::trace(&try_catch);
		return JS_undefined;
	}
	Handle<Value> result = script->Run();
	if (result.IsEmpty()) {
		// Print errors that happened during execution.
		k7::trace(&try_catch);
		return JS_undefined;
	}
	return result;
}

Handle<Object> k7::module(const char* fullName) {
	return k7::module(JS_GLOBAL, fullName, NULL);
}

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
			if (!module->Has(v8::String::New("__module__"))) {
				module->Set (v8::String::New("__module__"), v8::String::New(fullName));
			}
		}
	} else {
		// We only set the __module__ slot for the module we are loading.
		module = Object::New();
		if (rest == NULL) {
			module->Set(v8::String::New("__module__"), v8::String::New(fullName));
		}
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

Handle<String> k7::load(const char* path) {
	FILE* file = fopen(path, "rb");
	if (file == NULL) return v8::Handle<v8::String>();

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

	// Create a template for the global object.
	Handle<ObjectTemplate> global_template = ObjectTemplate::New();

	// Create a new execution environment containing the built-in
	// functions
	Handle<Context> context = Context::New(NULL, global_template);
	Handle<Object>  global  = context->Global();
  

	// Enter the newly created execution environment.
	Context::Scope context_scope(context);

	k7::setup(context->Global(), argc, argv, env);
	EXEC("system.k7.shell.run('pouet.js');");

	return 0;
}

// ----------------------------------------------------------------------------
//
// THE C MAIN
//
// ----------------------------------------------------------------------------

#ifndef __K7_LIBRARY_ONLY__
int main (int argc, char **argv, char **env) {
	return k7::main(argc, argv, env);
}
#endif

// EOF - vim: ts=4 sw=4 noet
