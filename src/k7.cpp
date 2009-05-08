// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
// Author            : Isaac Schulter.                           <i@foohack.com>
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

// This does the actual loading of modules
ENVIRONMENT
{
	LOAD("system.k7.modules",      system_k7_modules);
	LOAD("system.k7.shell",        system_k7_shell);
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
}
END

// ----------------------------------------------------------------------------
//
// MAIN SHELL FUNCTIONS
//
// ----------------------------------------------------------------------------

void k7_reportException (const TryCatch* try_catch) {
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

// Executes a string within the current v8 context.
bool k7_evalString (Handle<String> source, Handle<Value> fromFileName) {
	
	if (source->Length() == 0) return true;
	
	HandleScope handle_scope;
	TryCatch try_catch;
	
	// comment the #! line if one exists.
	String::Utf8Value utf8_value(source);
	if (
		(*utf8_value)[0] == '#' &&
		(*utf8_value)[1] == '!'
	) {
		(*utf8_value)[1] = (*utf8_value)[0] = '/';
		source = String::New(*utf8_value);
	}
	
	Handle<Script> script = Script::Compile(source, fromFileName);
	if (script.IsEmpty()) {
		// Print errors that happened during compilation.
		k7_reportException(&try_catch);
		return false;
	}
	Handle<Value> result = script->Run();
	if (result.IsEmpty()) {
		// Print errors that happened during execution.
		k7_reportException(&try_catch);
		return false;
	}
	return true;
}

int k7_main (int argc, char **argv, char **env) {

	HandleScope handle_scope;

	// Create a template for the global object.
	Handle<ObjectTemplate> global = ObjectTemplate::New();

	// Create a new execution environment containing the built-in
	// functions
	Handle<Context> context = Context::New(NULL, global);
	// Enter the newly created execution environment.
	Context::Scope context_scope(context);

	SetupEnvironment(context->Global(), argc, argv, env);
	
	/*
	bool read_stdin = true;
	bool read_string = false;
	bool process_flags = true;
	bool process_single = false;
	for (int i = 1; i < argc; i++) {
		const char* str = argv[i];
		
		if (read_string) {
			read_string = false;
			read_stdin = false;
			process_flags = false;
			if (!ExecuteString(JS_str(str), JS_str("[command line arg]"), false)) return 1;
			if (process_single) break;
		} else if (str[0] != '-') {
			// Use all other arguments as names of files to load and run.
			// If there are any explicit files, then don't read stdin.
			// @TODO: Each file should have an easy way to access the 
			// argv that came after its name.  In other words, something like this:
			// k7 file1.js -foo file1.js -bar
			// should be roughly similar to:
			// k7 file1.js -foo; k7 file1.js -bar;
			// except that in the first case, they share a global context.
			read_stdin = false;
			process_flags = false;
			if (!ExecuteFile(str)) return 1;
			if (process_single) break;
		} else if (process_flags && str[0] == '-') {
			// is a flags!
			if (strcmp(str, "--") == 0) {
				process_flags = false;
			} else if (strcmp(str, "-e") == 0) {
				read_string = true;
			} else if (strcmp(str, "-s") == 0 || strcmp(str, "--single") == 0) {
				process_single = true;
			} else if (
				strcmp(str, "-?") == 0 || strcmp(str, "-h") == 0 || strcmp(str, "--help") == 0
			) {
				read_stdin = false;
				printf(k7::help(), V8::GetVersion(), argv[0]);
				return 0;
			}
		}
		
	}
	if (read_stdin && !ExecuteFile("/dev/stdin")) return 1;
	*/
	return 0;
}

#ifndef __K7_LIBRARY_ONLY__
int main (int argc, char **argv, char **env) {
	return k7_main(argc, argv, env);
}
#endif

// EOF - vim: ts=4 sw=4 noet
