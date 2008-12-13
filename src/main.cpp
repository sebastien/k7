// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
// ----------------------------------------------------------------------------
// Creation date     : 27-Sep-2008
// Last modification : 27-Sep-2008
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
IMPORT(net_http_server_shttpd)
IMPORT(net_http_server_fcgi)

ENVIRONMENT
	LOAD("system.posix",          system_posix);
	LOAD("net.http.server.shttpd",net_http_server_shttpd);
	LOAD("net.http.server.fcgi",  net_http_server_fcgi);
END

// ----------------------------------------------------------------------------
//
// BASIC SHELL FUNCTIONS
//
// ----------------------------------------------------------------------------

bool ExecuteString(v8::Handle<v8::String> source,
                   v8::Handle<v8::Value> name,
                   bool print_result);

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

v8::Handle<v8::String> ReadFile(const char* name) {
  FILE* file = fopen(name, "rb");
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

// Executes a string within the current v8 context.
bool ExecuteString(v8::Handle<v8::String> source,
                   v8::Handle<v8::Value> name,
                   bool print_result) {
  v8::HandleScope handle_scope;
  v8::TryCatch try_catch;
  v8::Handle<v8::Script> script = v8::Script::Compile(source, name);
  if (script.IsEmpty()) {
    // Print errors that happened during compilation.
    v8::String::AsciiValue error(try_catch.Exception());
    printf("%s\n", *error);
    return false;
  } else {
    v8::Handle<v8::Value> result = script->Run();
    if (result.IsEmpty()) {
      // Print errors that happened during execution.
      v8::String::AsciiValue error(try_catch.Exception());
      printf("%s\n", *error);
      return false;
    } else {
      if (print_result && !result->IsUndefined()) {
        // If all went well and the result wasn't undefined then print
        // the returned value.
        v8::String::AsciiValue str(result);
        printf("%s\n", *str);
      }
      return true;
    }
  }
}

// ----------------------------------------------------------------------------
//
// MAIN SHELL FUNCTIONS
//
// ----------------------------------------------------------------------------

int main(int argc, char ** argv) {
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
	SetupEnvironment(context->Global());

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
			v8::Handle<v8::String> source = ReadFile(str);
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
