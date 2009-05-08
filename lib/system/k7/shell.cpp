// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
// ----------------------------------------------------------------------------
// Creation date     : 08-May-2008
// Last modification : 08-May-2009
// ----------------------------------------------------------------------------

#include "macros.h"

// functions that will be used later on
Handle<Object> JSEnv (int, char **, char **);
bool ExecuteString (Handle<String>, Handle<Value>, bool);
bool ExecuteFile (FILE *, const char *);
bool ExecuteFile (const char *);
bool ExecuteFile (Handle<Value>, const char *);
void ReportException(const TryCatch *);

Handle<Value> ReadFile (const char*);
Handle<Value> ReadFile (FILE*);

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
