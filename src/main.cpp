// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
// ----------------------------------------------------------------------------
// Creation date     : 27-Sep-2008
// Last modification : 19-Mar-2009
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

IMPORT(system_posix)
IMPORT(system_k7_modules)
IMPORT(net_http_server_shttpd)
IMPORT(data_formats_json)
#ifdef WITH_FCGI
	IMPORT(net_http_server_fcgi)
#endif
#ifdef WITH_CURL
	IMPORT(net_http_client_curl)
#endif

ENVIRONMENT
{
	#include "core.h"
	EVAL(CORE_JS)

	LOAD("system.posix",          system_posix);
	//LOAD("system.k7.modules",     system_k7_modules);
	LOAD("data.formats.json",     data_formats_json);
	LOAD("net.http.server.shttpd",net_http_server_shttpd);
#ifdef WITH_FCGI
	LOAD("net.http.server.fcgi",  net_http_server_fcgi);
#endif
#ifdef WITH_CURL
	LOAD("net.http.client.curl",  net_http_client_curl);
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
//
FUNCTION(Print)
	bool first = true;
	for (int i = 0; i < args.Length(); i++) {
		v8::HandleScope handlescope;
		if (first) { first = false; }
		else       { printf(" "); }
		ARG_str(str,i);
		printf("%s", *str);
	}
	printf("\n");
	return JS_undefined;
END

Handle<Value> ReadFile (FILE *file) {
	if (
		file == NULL
	) return ThrowException(String::New("Could not read file"));
	
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

void ReportException(TryCatch* try_catch) {
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
	
	// remove the #! line if one exists.
	String::Utf8Value utf8_value(source);
	char *tmp = new char[source->Length()];
	strcpy(tmp, *utf8_value);
	int shebanger = 0, size = strlen(tmp);
	if (tmp[0] == '#' && tmp[1] == '!') {
		while (tmp[shebanger] != '\n' && tmp[shebanger] != '\0') shebanger ++;
		source = String::New(tmp+shebanger, size-shebanger);
	}
	delete[] tmp;
	
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

int main(int argc, char** argv, char** env) {
	v8::V8::SetFlagsFromCommandLine(&argc, argv, true);
	v8::HandleScope handle_scope;

	// Create a template for the global object.
	v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New();

	// Create a new execution environment containing the built-in
	// functions
	v8::Handle<v8::Context> context = v8::Context::New(NULL, global);
	// Enter the newly created execution environment.
	v8::Context::Scope context_scope(context);
	context->Global()->Set(JS_str("print"), v8::FunctionTemplate::New(Print)->GetFunction()); \
	SetupEnvironment(context->Global(), argc, argv, env);

	bool run_shell = (argc == 1);
	for (int i = 1; i < argc; i++) {
		const char* str = argv[i];
		if (strcmp(str, "--shell") == 0) {
			run_shell = true;
		} else if (strncmp(str, "--", 2) == 0) {
			printf("Warning: unknown flag %s.\n", str);
		} else {
			// Use all other arguments as names of files to load and run.
			v8::HandleScope handle_scope;
			v8::Handle<v8::String> file_name = v8::String::New(str);
			v8::Handle<v8::String> source = ReadFile(str)->ToString();
			context->Global()->Set(JS_str("__file__"), file_name);
			if (source.IsEmpty()) {
				printf("Error reading '%s'\n", str);
				return 1;
			}
			if (!ExecuteString(source, file_name, false))
				return 1;
		}
	}
	if (run_shell) RunShell(context);
	return 0;
}

// EOF - vim: ts=4 sw=4 noet
