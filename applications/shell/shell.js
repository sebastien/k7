#!/usr/bin/k7
(function () {

print.helpText = "Print out the arguments, separated by spaces.";
include.helpText = "Load a javascript file.";
// evalcx.helpText = "Evaluate code in a sandboxed context. Params: string code, object sandbox, boolean saveState";



var dirname = function (f) {
	if (f[0] !== '/') f = './' + f;
	f = f.split('/');
	f.pop();
	return f.join('/');
};

// a function to list stuff
dir = function (obj, ret, prefix) {
	prefix=prefix||'';
	var s = [prefix + obj];
	for (var i in obj) if (obj[i] !== undefined) {
		s.push(prefix+i+":"+' '+obj[i]);
		// if (obj[i] && (typeof(obj[i]) === 'object')) {
		// 	s.push(dir(obj[i],true,prefix+i+'.'));
		// }
	}
	s = s.join('\n');
	if (ret) return s;
	return print(s, "\n");
};
dir.helpText = "Print out all the members in an object";

var P = system.posix;
var getLine = function (file, delim) {
	delim = delim || '\n';
	var dl = delim.length;
	var str = P.fread(file, 1);
	var data = str;
	while ( data.substr(-1*dl, dl) !== delim && str ) {
		var str = P.fread(file, 1);
		data += str;
	}
	return data;
};

print(["",
	"k7 Shell",
	"^D or quit() to exit",
	"help() for help",
	""].join("\n"));

help = function () { 
	var commands = ["","Shell Commands:", "------------------------------------"];
	var modules = ["","","Objects:", "------------------------------------"];
	for (var i in this) {
		if (typeof(this[i]) === 'function') {
			commands.push(i+"()  "+(
				this[i].helpText || this[i]
			));
		} else if (this[i] !== undefined) {
			modules.push(i + '   ' + this[i]);
		}
	}
	print(commands.join('\n'));
	print(modules.join('\n'));
	print("\n");
	print(["",
		"Most functions return true if they succeed, so you see a lot of \"true\" stuff. That's normal.",""
	].join("\n"));
	return true;
}
help.helpText = "This helpful list.";

var prompt = (function () {
	var go = true;
	quit = function () {
		go = false;
		print("\nquitting...");
		return true;
	};
	return function () {
		if (go) print("\nk7sh> ");
		return go;
	};
})();
quit.helpText = "Quit the shell.";

var shell_print = function (o) {
	print("\n>", o, "\n");
};

var str;
var stdin = P.fopen("/dev/stdin", "rb");
var ex;

// @TODO
// this global-wrapping business was a fun foray into
// an interesting intellectual exercise, but it's gone
// too far astray, and is clearly not useful.
// Better to investigate snapshots and handle the
// reset functionality that way.

while (prompt() && (str = getLine(stdin))) try {
	if (str != "\n") shell_print(eval(str));
} catch (ex) {
	shell_print(ex);
}
if (!str) shell_print(quit());

P.fclose(stdin);

return true;

})();