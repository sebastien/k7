system.k7.modules.update(system.k7.shell,{
	command:function() {
		var ENV   = system.k7.ENV
		var shell = system.k7.shell
		var print = shell.print
		var posix = system.posix
		// We enter interactive mode
		if (ENV.argc == 1) {
			print("Usage: k7 SOURCE\nNOTE: Interactive mode not supported yet");
		} else {
		// We run the given file
			var source = shell.run(ENV.argv[1])
		}
	}
})
