// load jaspers outside of the fcgi accept() loop.

String.prototype.trim = function() {
	return this.replace(/^\s+|\s+$/g, '');
};

(function () {
	var f = new net.http.server.fcgi.FCGI();
	
	this.print = function ( /* arg, arg, arg */ ) {
		var s = [];
		for ( var i = 0, l = arguments.length; i < l; i ++ ) if ((''+arguments[i]).length) {
			s.push(''+arguments[i]);
		}
		if (!s.length) return;
		if (!sentHeaders) sendHeaders();
		f.putstr(s.join(' '));
	};
	
	this.errorLog = function ( /* arg, arg, arg */ ) {
		var s = [];
		for ( var i = 0, l = arguments.length; i < l; i ++ ) if ((''+arguments[i]).length) {
			s.push(''+arguments[i]);
		}
		s.push("\n");
		if (s.length) f.errorLog(s.join(' '));
	};
	
	/**
	 * Set the get vars on the server object
	 */
	var setRequestVars = function() {

		var tokens = this.FCGI_ENV.QUERY_STRING.split('&'),
			value;
		
		this.server.get = qsToObj(this.FCGI_ENV.QUERY_STRING);
		this.server.post = qsToObj(this.getRawRequest());
		
	};
	var qsToObj = function (qs) {
		var obj = {};
		qs = (qs+'').trim().replace(/\+/g, ' ').split('&');
		
		for (var i = 0, l = qs.length; i < l; i++) if (qs[i].trim().length) {
			var toks = qs[i].trim().split('=');
			obj[toks.shift()] = decodeURIComponent(toks.join('='));
		}
		return obj;
	};
	
	
	var sendHeaders = function () {
		sentHeaders = true;
		for (var i in headers) if (headers[i] !== null) {
			f.putstr(i + ": " + headers[i] + "\r\n");
		}
		f.putstr("\r\n");
	};
	
	/**
	 * Send a list of strings to the output buffer
	 */
	this.print = function ( /* arg, arg, arg */ ) {
		var s = [];
		for ( var i = 0, l = arguments.length; i < l; i++ ) if (('' + arguments[i]).length) {
			s.push('' + arguments[i]);
		}
		if (!s.length) return;
		if (!sentHeaders) sendHeaders();
		f.putstr(s.join(''));
	};
	
	this.header = function (k, v) {
		if (sentHeaders) return;
		headers[k] = v;
	};

	while (f.accept() >= 0) {
		
		this.server = {
			get : {},
			post : {}
		};

		this.FCGI_ENV = f.getEnv();

		var sentHeaders = false;
		var headers = {
			"Content-type" : "text/html; charset=utf-8"
		};
		
		// trap it so that it's unique to each request.
		this.getRawRequest = function (r) {
			return function () {
				return r;
			};
		}(f.getRawRequest());
		

		setRequestVars();
		// @TODO: Replace this ugly evalcx hack with a cleaner
		// system.engine.getSnapshot implementation.
		try {
			evalcx(this.system.posix.readFile(this.FCGI_ENV.SCRIPT_FILENAME), this);
		} catch (ex) {
			var message = ex.line + " " + ex.message;
			errorLog(message);
			print(message);
		}
	}
	
	f.free();
})();
