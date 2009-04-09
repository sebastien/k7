// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
// Author            : Isaac Schulter.                           <i@foohack.com>
// ----------------------------------------------------------------------------
// Creation date     : 27-Sep-2008
// Last modification : 31-Mar-2009
// ----------------------------------------------------------------------------

#include <v8.h>
#include <k7.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

using namespace v8;

// ----------------------------------------------------------------------------
//
// K7 ENVIRONMENT
//
// ----------------------------------------------------------------------------

// functions that will be used later on
Handle<Object> JSEnv (int, char **, char **);
bool ExecuteString (Handle<String>, Handle<Value>, bool);
bool ExecuteFile (FILE *, const char *);
bool ExecuteFile (const char *);
bool ExecuteFile (Handle<Value>, const char *);
void ReportException(const TryCatch *);

Handle<Value> ReadFile (const char*);
Handle<Value> ReadFile (FILE*);

// import libs from the "libs" folder.
IMPORT(system_posix);
IMPORT(system_k7_modules);
IMPORT(system_engine);
IMPORT(data_formats_json);
IMPORT(net_http_server_shttpd);
#ifdef WITH_FCGI
	IMPORT(net_http_server_fcgi);
#endif
#ifdef WITH_CURL
	IMPORT(net_http_client_curl);
#endif

// functions that will be put on the global object.
FUNCTION_DECL(Print);
FUNCTION_DECL(Load);
FUNCTION_DECL(EvalCX);

// define the "SetupEnvironment" function
ENVIRONMENT
{
	JSOBJ_set(global,"ENV",JSEnv(argc, argv, env));
	JSOBJ_set(global,"print", JS_fn(Print));
	JSOBJ_set(global,"include", JS_fn(Load));
	JSOBJ_set(global,"evalcx", JS_fn(EvalCX));
	
	#include "core.h"
	EVAL(CORE_JS)

	LOAD("system.posix", system_posix);
	LOAD("system.engine", system_engine);
	LOAD("system.k7.modules", system_k7_modules);
	LOAD("data.formats.json", data_formats_json);
	LOAD("net.http.server.shttpd",net_http_server_shttpd);
#ifdef WITH_FCGI
	LOAD("net.http.server.fcgi", net_http_server_fcgi);
#endif
#ifdef WITH_CURL
	LOAD("net.http.client.curl", net_http_client_curl);
#endif
}
END

// ----------------------------------------------------------------------------
//
// BASIC SHELL FUNCTIONS
//
// ----------------------------------------------------------------------------
// The callback that is invoked by v8 whenever the JavaScript 'print'
// function is called.  Prints its arguments on stdout separated by
// spaces and ending with a newline.

Handle<Object> JSEnv (int argc, char **argv, char **env) {
	Handle<Object> jsenv = JS_obj();
	
	for (int i = 0; env[i]; i ++) {
		int j;
		for (j = 0; env[i][j] && env[i][j] != '='; j ++);
		env[i][j] = '\0';
		jsenv->Set(JS_str(env[i]), JS_str(env[i]+j+1));
	}

	jsenv->Set(JS_str("argc"), JS_int(argc));

	Handle<Array> jsargv = Array::New(argc);
	for (int i = 0; i < argc; i ++) {
		jsargv->Set(JS_int(i), JS_str(argv[i])); 
	}
	jsenv->Set(JS_str("argv"), jsargv);

	return jsenv;
}

Handle<Value> ReadFile (FILE *file) {
	if (
		file == NULL
	) return ThrowException(Exception::Error(String::New("Could not read file")));
	
	std::string data;
	char c;
	while ( (c = fgetc(file)) && !feof(file) ) {
		data.push_back(c);
	}
	
	return String::New(data.c_str(), strlen(data.c_str()));	
}

Handle<Value> ReadFile (const char* name) {
	return ReadFile(fopen(name, "rb"));
}


FUNCTION(Print)
	bool first = true;
	for (int i = 0; i < args.Length(); i++) {
		if (first) {
			first = false;
		} else {
			printf(" ");
		}
		ARG_str(str,i);
		printf("%s", *str);
	}
	return Boolean::New(true);
END

void copy_obj( Handle<Object> from, Handle<Object> to)
{
	Handle<Array> keys = from->GetPropertyNames();
	for (int i = 0; i < keys->Length(); i ++) {
		Handle<String> key = keys->Get(JS_int(i))->ToString();
		Handle<Value> val = from->Get(key);
		to->Set(key, val);
	}
}


// @TODO: Move this to system.engine
static Handle<Value> EvalCX (const Arguments& args)
{
	HandleScope handle_scope;
	Handle<Context> context = Context::New();
	context->SetSecurityToken(
		Context::GetCurrent()->GetSecurityToken()
	);
	
	Context::Scope context_scope(context);
	Handle<String> code = args[0]->ToString();
	Handle<Object> sandbox = args.Length() >= 1 ?
		args[1]->ToObject() : Context::GetCurrent()->Global();
	
	// share global datas
	copy_obj( sandbox, context->Global());
	context->Enter();
	
	TryCatch try_catch;
	Handle<Script> script = Script::Compile(code, String::New("evalcx"));
	Handle<Value> result;
	if (script.IsEmpty()) {
		ReportException(&try_catch);
		result = ThrowException(try_catch.Exception());
		goto CX_CLEANUP; // no run for you!
	}
	
	result = script->Run();
	if (result.IsEmpty()) {
		ReportException(&try_catch);
		result = ThrowException(try_catch.Exception());
		goto CX_CLEANUP; // no global copy for you!
	}
	
	if (args.Length() >= 3 && args[2]->IsTrue()) {
		copy_obj( context->Global(), sandbox );
	}
	
CX_CLEANUP:
	// Because the global is about to be destroyed,
	// if the code ended in some reference to the global
	// it'll be "null" in the calling code, when the
	// more intuitive intent would be to return a reference
	// to the sandbox object.
	if (result == context->Global()) {
		result = sandbox;
	}
	context->DetachGlobal();
	context->Exit();
	return result;
}


void ReportException(const TryCatch* try_catch) {
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
bool ExecuteString (
	Handle<String> source,
	Handle<Value> name,
	bool print_result
) {
	
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
	
	Handle<Script> script = Script::Compile(source, name);
	if (script.IsEmpty()) {
		// Print errors that happened during compilation.
		ReportException(&try_catch);
		return false;
	}
	Handle<Value> result = script->Run();
	if (result.IsEmpty()) {
		// Print errors that happened during execution.
		ReportException(&try_catch);
		return false;
	}
	
	if (print_result) {
		// print the returned value.
		String::AsciiValue str(result);
		
		printf("%s\n", *str);
	}
	return true;
}

// Expose Read-and-execute in the JS environment
FUNCTION(Load)
{	
	Handle<Value> retval = Undefined();
	Local<String> includer(Context::GetCurrent()->Global()->Get(JS_str("__file__"))->ToString());
	HandleScope handle_scope;
	TryCatch try_catch;
	
	for (int i = 0; i < args.Length(); i++) {
		String::Utf8Value file(args[i]);
		if (*file == NULL) {
			retval = ThrowException(Exception::Error(String::New("Error loading file")));
			goto LOAD_CLEANUP;
		}
		
		Handle<String> source = ReadFile(*file)->ToString();
		if (source.IsEmpty()) {
			retval = ThrowException(Exception::Error(String::New("Error loading file")));
			goto LOAD_CLEANUP;
		}
		
		// set the __file__ value.
		Context::GetCurrent()->Global()->Set(JS_str("__file__"), JS_str(*file));
		if (!ExecuteString(source, String::New(*file), false)) {
			retval = ThrowException(try_catch.Exception());
			goto LOAD_CLEANUP;
		}
	}
	
LOAD_CLEANUP:
	Context::GetCurrent()->Global()->Set(JS_str("__file__"), includer);
	return retval;
}
END


bool ExecuteFile (FILE *file, const char *str) {
	return ExecuteFile(ReadFile(file), str);
}

bool ExecuteFile (const char *str) {
	// handle some "special" filenames a little differently.
	// Just stdin for now, but there may be others, I suppose.
	return 
		strcmp(str, "/dev/stdin") == 0 ? ExecuteFile(stdin, "/dev/stdin")
		: ExecuteFile(ReadFile(str), str);
}

bool ExecuteFile (Handle<Value> read_value, const char *str) {
	
	HandleScope handle_scope;
	
	Handle<String> file_name = String::New(str);
	Handle<String> source = read_value->ToString();
	
	JSOBJ_set(Context::GetCurrent()->Global(),"__file__", file_name);
	
	return (!source.IsEmpty() && ExecuteString(source, file_name, false));
}


// The read-eval-execute loop of the shell.
void RunShell(v8::Handle<v8::Context> context) {

	printf("K7/V8 version %s\n", v8::V8::GetVersion());
	static const int kBufferSize = 256;

	while (true) {
		char buffer[kBufferSize];
		printf("> ");
		char* str = fgets(buffer, kBufferSize, stdin);
		if (str == NULL) break;
		v8::HandleScope handle_scope;
		ExecuteString(v8::String::New(str), v8::Undefined(), true);
	}

	printf("\n");
}

// ----------------------------------------------------------------------------
//
// MAIN SHELL FUNCTIONS
//
// ----------------------------------------------------------------------------

namespace k7 {
	
	const char * help () {
		return "\nK7, using V8 version %s\n"
		"\n"
		"Usage:\n"
		"%s [flags] [filename | -e <js_string> [filename | -e <js_string> [...]]]\n"
		"\n"
		"FLAGS\n"
		"\n"
		"-?, -h\n"
		"  show this help\n"
		"-s, --single\n"
		"  Only parse a single file.  This is useful for javascript programs\n"
		"  that take the name of a file as a command line argument, to prevent\n"
		"  k7 from trying to parse that file as well.\n"
		"--\n"
		"  Stop parsing flags.  This is useful when a javascript program\n"
		"  receives command line flags such as -?, but does not want to have k7\n"
		"  try to interpret those flags.\n"
		"\n"
		"CODE\n"
		"One or more code segments can be specified, either via filenames or\n"
		"literal Javascript strings.  If no code is provided, k7 will read\n"
		"from the stdin stream.  If any executed code throws an error or cannot\n"
		"be parsed, k7 will die in failure.  Once a filename or -e string is\n"
		"encountered, no further flags are parsed by k7.\n"
		"\n"
		"-e <string>\n"
		"  execute <string>.  The \"filename\" of this run is set to\n"
		"  \"[command line arg]\" in k7's environment.\n"
		"filename\n"
		"  Pass in the names of one or more files.\n"
		"  If no files or -e strings are specified, then stdin is read.\n"
		"  The filename \"/dev/stdin\" is special, and will cause\n"
		"  k7 to read off the stdin stream, even on systems (like Windows)\n"
		"  that don't alias the standard input to this filename.\n"
		"  Filenames may not start with a -.  If the filename does\n"
		"  start with a -, then refer to it as \"./-file\" or something.\n"
		"\n"
		"Any unrecognized flags will be ignored. Scripts may use these,\n"
		"however, by investigating the global ENV.argv array.\n"
		"\n"
		"Contributions welcome! <http://github.com/isaacs/k7>\n";
	}
	
	int main (int argc, char **argv, char **env) {

		Locker lock;
		HandleScope handle_scope;

		// Create a template for the global object.
		Handle<ObjectTemplate> global = ObjectTemplate::New();

		// Create a new execution environment containing the built-in
		// functions
		Handle<Context> context = Context::New(NULL, global);
		// Enter the newly created execution environment.
		Context::Scope context_scope(context);
	
		SetupEnvironment(context->Global(), argc, argv, env);
		
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
		
		return 0;
	}
}

#ifndef __K7_LIBRARY_ONLY__
int main (int argc, char **argv, char **env) {
	return k7::main(argc, argv, env);
}
#endif

// EOF - vim: ts=4 sw=4 noet
