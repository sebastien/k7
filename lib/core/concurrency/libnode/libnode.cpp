// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
// ----------------------------------------------------------------------------
// Creation date     : 03-Oct-2009
// Last modification : 03-Oct-2009
// ----------------------------------------------------------------------------

#ifdef WITH_LIBNODE
#include <k7.h>

#include "node/deps/evcom/evcom.h"
#include "node/deps/libeio/eio.h"
#include "node/src/node.h"
#include "node/src/node_stdio.h"
#include "node/src/events.h"
#include "node/src/timer.h"
#include "node/src/child_process.h"
#include "node/src/constants.h"
#include "node/src/dns.h"
#include "node/src/file.h"
#include "node/src/http.h"

#define MODULE_NAME   "core.concurrency.libnode"
#define MODULE_STATIC  core_concurrency_libnode

FUNCTION(node_runLoop) {
  ev_loop(EV_DEFAULT_UC_ 0);
} END

static ev_async eio_watcher;
static void EIOCallback(EV_P_ ev_async *watcher, int revents) {
  assert(watcher == &eio_watcher);
  assert(revents == EV_ASYNC);
  eio_poll();
}

static void EIOWantPoll(void) {
  ev_async_send(EV_DEFAULT_UC_ &eio_watcher);
}

MODULE
{
	// FIXME: We should make sure this is only initialized once per process
	evcom_ignore_sigpipe();
	ev_default_loop(EVFLAG_AUTO);
	ev_async_init(&eio_watcher, EIOCallback);
	eio_init(EIOWantPoll, NULL);
	ev_async_start(EV_DEFAULT_UC_ &eio_watcher);
	ev_unref(EV_DEFAULT_UC);

	Local<FunctionTemplate> process_template = FunctionTemplate::New();
	// The global object / "process" is an instance of EventEmitter.  For
	// strange reasons we must initialize EventEmitter now!  it will be assign
	// to it's namespace node.EventEmitter in Load() bellow.
	node::EventEmitter::Initialize(process_template);
	Persistent<Context> context = Context::New(NULL, process_template->InstanceTemplate());
	//Context::Scope context_scope(context);

	// Sets up Node's globals
	SET("process",      context->Global());
	SET("EventEmitter", node::EventEmitter::constructor_template->GetFunction());

	// Initializing the various modules
	node::Promise::Initialize(self);
	node::Stdio::Initialize(self);
	node::Timer::Initialize(self);
	node::ChildProcess::Initialize(self);
	node::DefineConstants(self);

	Local<Object> dns = Object::New();
	SET("dns", dns);
	node::DNS::Initialize(dns);

	Local<Object> fs = Object::New();
	SET("fs", fs);
	node::File::Initialize(fs);

	Local<Object> tcp = Object::New();
	SET("tcp", tcp);
	node::Server::Initialize(tcp);
	node::Connection::Initialize(tcp);

	Local<Object> http = Object::New();
	SET("http", http);
	node::HTTPServer::Initialize(http);
	node::HTTPConnection::Initialize(http);

	BIND("runLoop", node_runLoop);

	// We load the JavaScript runtime
	#include "libnode.js.h"
	EXEC(LIBNODE_JS)
}
END

#endif
// EOF - vim: ts=4 sw=4 noet
