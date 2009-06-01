/**
Demonstrates the use of the 'libtask.yield' function. Note that if there is an
exception with libtask, K7 will segfault.
--
Task 1:0
Task 2:0
Task 1:1
Task 2:1
Task 1:2
Task 2:2
Task 1:3
Task 2:3
Task 1:4
Task 2:4
Task 1:5
Task 2:5
Task 1:6
Task 2:6
Task 1:7
Task 2:7
Task 1:8
Task 2:8
Task 1:9
Task 2:9
--
*/

var libtask = core.concurrency.libtask
var print   = system.shell.print;

function task_1 () {
	for (var i=0;i<10;i++) {
		print("Task 1:" + i + "\n")
		libtask.yield()
	}
}

function task_2 () {
	for (var i=0;i<10;i++) {
		print("Task 2:" + i + "\n")
		libtask.yield()
	}
}

function main () {
	libtask.create(task_1,[])
	libtask.create(task_2,[])
	libtask.create(task_2,[])
	libtask.create(task_2,[])
	libtask.create(task_2,[])
	libtask.create(task_2,[])
	libtask.create(task_2,[])
	libtask.create(task_2,[])
	libtask.create(task_2,[])
	libtask.create(task_2,[])
}

main()
