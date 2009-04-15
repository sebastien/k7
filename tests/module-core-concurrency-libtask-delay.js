var libtask = core.concurrency.libtask

function task_1 () {
	for (var i=0;i<10;i++) {
		print("Task 1:" + i)
		libtask.yield()
	}
}

function task_2 () {
	for (var i=0;i<10;i++) {
		print("Task 2:" + i)
		libtask.yield()
	}
}

function main () {
	libtask.create(task_1,[])
	libtask.create(task_2,[])
}

main()
