#include <v8.h>
#include <stdio.h>

using namespace v8;

void SetupBuiltIns (Handle<Object> target) {
	printf("Initializing builtins...\n");
}

int main(int argc, char ** argv) {

	// initialize
	v8::V8::SetFlagsFromCommandLine(&argc, argv, true);
	v8::HandleScope handle_scope;

	// bind functions
	v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New();

	v8::Handle<v8::Context> context = v8::Context::New(NULL, global);
	v8::Context::Scope context_scope(context);

	// context->Global()->Set(v8::String::New("DLL"), setup_v8ext());
	SetupBuiltIns(context->Global());

	if (argc >= 2) {
		const char *srcfile = argv[1];
		v8::Handle<v8::String> file_name = v8::String::New(srcfile);
		v8::Handle<v8::String> source = v8::String::New("var a=1;");
		if (source.IsEmpty()) {
			printf("Error reading '%s'\n", srcfile);
			return 1;
		}
		/*if (!Exec(source, file_name, false)) {
			return 1;
		}
		*/
		return 0;
	} else {
		// run in shell mode
		return 0;
	}
}

// EOF - vim: ts=4 sw=4 noet
