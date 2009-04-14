#!/bin/bash

cleanup () {
	rm compile{,.cpp} hello/hello{,.cpp} &> /dev/null
}
runtest () {
	echo $1
	echo $2
	[ "$2" == "$3" ] && echo "Success!" || echo "fail :("
	cleanup
}

runtest \
	"test 1 - compile and run hello.js" \
	"$( ./compile.js hello/hello.js && hello/hello )" \
	"$( k7 hello/hello.js )"

runtest \
	"test 2 - compile and run compile.js, then compile hello.js using the compiled compiler." \
	"$( ./compile.js compile.js && ./compile hello/hello.js && hello/hello )" \
	"$( k7 hello/hello.js )"

runtest \
	"test 3 - compile compile.js using the compiled compiler, then compile hello.js with the new compiler." \
	"$( ./compile.js compile.js && mv compile compile_ && ./compile_ compile.js && rm compile_ && ./compile hello/hello.js && hello/hello )" \
	"$( k7 hello/hello.js )"
