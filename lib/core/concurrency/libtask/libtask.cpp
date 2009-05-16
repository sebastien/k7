// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
// ----------------------------------------------------------------------------
// Creation date     : 15-Apr-2009
// Last modification : 10-May-2009
// ----------------------------------------------------------------------------

#ifdef WITH_LIBTASK
#include <k7.h>
#include "libtask/task.h"

#define MODULE_NAME   "core.concurrency.libtask"
#define MODULE_STATIC  core_concurrency_libtask

// Libtask documentation is available at
// <http://swtch.com/libtask/>

FUNCTION_C(libtask_system)
{
	ARG_COUNT(0);
	tasksystem();
}
END

FUNCTION_C(libtask_yield)
{
	ARG_COUNT(0);
	// As noted here <http://ur1.ca/3dp3>
	// No HandleScope and not Context should be active when yielding
	taskyield();
}
END

FUNCTION_C(libtask_delay)
{
	ARG_COUNT(1);
	ARG_int(status,0);
	taskdelay((unsigned int)status);
}
END

FUNCTION_C(libtask_id)
{
	ARG_COUNT(1);
	ARG_int(status,0);
	return JS_int(taskid());
}
END

FUNCTION_C(libtask_exit)
{
	ARG_COUNT(1);
	ARG_int(status,0);
	taskexit(status);
}
END

FUNCTION_C(libtask_exitAll)
{
	ARG_COUNT(1);
	ARG_int(status,0);
	taskexitall(status);
}
END

// NOTE: This is WIP code that will be moved to a "task" module using libtask API
void libtask_create_helper(void* context) {
	//ARG_COUNT(1);
	//ARG_obj(callback_context,0);

	HandleScope HandleScope;
	Object* context_obj = reinterpret_cast<Object*>(context);
	Handle<Function> callback_v = Handle<Function>::Cast(context_obj->Get(JS_str("callback")));
	callback_v->Call(callback_v, 0, NULL);

	// TODO: We should release the callback context
}

OBJECT(libtask_TASK,0)
{
	return self;
}
END

FUNCTION_C(libtask_create)
{
	ARG_COUNT(2);
	//Handle<Function> callback = Handle<Function>::Cast(args[(0)]);
	ARG_fn(callback,0);
	ARG_obj(arguments, 1);
	//Persistent<Arguments> thread_arguments = args;
	// FIXME: This won't work, of course

	Persistent<Object> result = Persistent<Object>::New(libtask_TASK());
	result->Set(JS_str("callback"),callback);
	/* SEGFAULT HERE
	result->Set(JS_str("arguments"),arguments);
	*/
	// TODO: Use a global to setup tasksize
	int r = taskcreate(libtask_create_helper, (void*)*result, 32768);
	return JS_int(r);
}
END

MODULE
{
	BIND("create",    libtask_create);
	BIND("id",        libtask_id);
	BIND("yield",     libtask_yield);
}
END

#endif
// EOF - vim: ts=4 sw=4 noet
