var posix = load("system.posix")

for (var i=0 ; i<10 ; i++ ) {
	posix.pthread_create(
		function(){
			for (var j=0;j<10;j+=1) {
				print("Thread " + i + ": step " + j)
			}
		}
	)
}


