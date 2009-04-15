// run some code, and provide read-access to global env.
var wrap = (function () {
	var wrappers = [];
	var depth = 0;
	var wrappable = function (m) {
		return ((typeof(m) === 'object' || typeof(m) === 'function') && m);
	};
	var apply = function (fn, self, args) {
		return fn.apply(self, Array.prototype.slice.call(args,0));
	};
	var wrapObj = function (o) {
		var n = function () {};
		n.prototype = o;
		var w = new n();
		w.constructor = undefined;
		w.__proto__ = undefined;
		var str = ''+o;
		w.toString = function () {
			return str;
		};
		wrappers.push([o, w]);
		for (var i in o) if (wrappable(o[i])) w[i] = WRAP(o[i]);
		return w;
	};
	var wrapArr = function (a) {
		var r = [];
		wrappers.push([a, r]);
		for (var i = 0, l = a.length; i < l; i ++) {
			r.push(WRAP(a[i]));
		}
		return r;
	};
	var wrapFn = function (fn) {
		var f = function () {return apply(fn, this, arguments);};
		var str = '' + fn;
		f.toString = function () {
			return str;
		};
		wrappers.push([fn, f]);
		f.prototype = WRAP(fn.prototype);
		for (var i in fn) if (!(i in f)) f[i] = WRAP(fn[i]);
		return f;
	};
	
	var WRAP = function (o) {
		depth ++;
		for (var i = 0, l = wrappers.length; i < l; i ++) {
			if (wrappers[i][0] === o) return wrappers[i][1];
		}
		if (!wrappable(o)) return o;
		var w =
			(o instanceof Array) ? wrapArr(o) :
			(typeof(o) === 'object') ? wrapObj(o)
			: (typeof(o) === 'function') ? wrapFn(o)
			: o;
		
		depth --;
		if (depth === 0) wrappers = [];
		return w;
	};
	return WRAP;
})();