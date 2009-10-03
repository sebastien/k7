var print      = system.shell.print
var node       = core.concurrency.libnode
var resolution = node.dns.resolve4("k7js.org")
resolution.addCallback(function(a,t,c){
	print ("DNS resolution of 'k7js.org': ", a, t, c)
})
// We don't forget to run node's event loop
node.runLoop()
