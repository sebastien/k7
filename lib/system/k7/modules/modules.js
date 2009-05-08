merge(system.k7.modules,{
	load:function(name){
		print("LOADING MODULE")
	},
	has:function(name){
		print("HAS MODULE")
	},
	update:function(module, values){
		for (var k in values) {
			module[k] = values[k];
		}
	}
})

