#!/usr/bin/k7 -s

// 
// usage: ./compile.js ./path/to/my/program.js
// 
// For now, it only compiles a single program into a binary.
// However, it'll have to support multi-file programs (as well as
// pre-compiling and other goodies) if it's ever going to be really
// useful.

(function () {
var argv = ENV.argv, argc = ENV.argc;
// remove k7 and -s
while (argv.length > 1 && argv[0] !== __file__) {
	argv.shift();
}
argc = argv.length;


for (var f = 0, fl = argv.length; f < fl; f ++) {

	var file = argv[f];
	var file = file.replace(/^\./, ENV.PWD);
	
	if (file[0] !== '/' && ENV.PWD[0] === '/') {
		file = ENV.PWD + "/" + file;
	}
	
	var js_file = file;
	
	var data = system.posix.readFile(js_file);

	var codes = [];
	for (var i = 0, l = data.length; i < l; i ++) {
		var b = data.charCodeAt(i);
		codes.push(b);
	}
	codes.push(0);

	var dirname = function (f) {
		if (f[0] !== '/') f = ENV.PWD + '/' + f;
		f = f.split('/');
		f.pop();
		return f.join('/');
	};
	var k7dir = dirname(dirname(dirname(__file__.replace(/^\./, ENV.PWD))));

	var bin = file.replace(/[\.][^\.]+$/, '');
	var cpp_file = bin+".cpp";
	system.posix.writeFile(cpp_file, [
		"#define __K7_LIBRARY_ONLY__ 1",
		"#include <stdlib.h>",
		"#include <stdio.h>",
		"#include \""+k7dir+"/src/main.cpp\"",
		"char code[] = {" + codes.join(", ") + "};",
		"int main (int argc, char **argv, char **env) {",
		"HandleScope handle_scope;",
		"Handle<ObjectTemplate> global = ObjectTemplate::New();",
		"Handle<Context> context = Context::New(NULL, global);",
		"Context::Scope context_scope(context);",
		"SetupEnvironment(context->Global(), argc, argv, env);",
		"context->Global()->Set(JS_str(\"__file__\"), JS_str(\"" + file + "\"));",
		"ExecuteString(JS_str(code), JS_str(\"compiled\"), false);",
		"return 0;",
		"}"
	].join("\n"));

	var cmd = "g++ "+ cpp_file +" "+k7dir+"/deps/v8/libv8.a -lcurl -lfcgi "+k7dir+"/deps/shttpd/src/libshttpd.a -lpthread -ldl -liconv `find "+k7dir+"/build -path */build/*/*.o -or -name modules.o` -o "+bin+" -I"+k7dir+"/src/ -I"+k7dir+"/deps/v8/include/";

	system.posix.system(cmd);
}


})();