/**
 * An interface to the javascript engine.
 * In k7, for now, this is v8.  However, in the future,
 * a different engine might be a better choice.
 * So, this module is not named v8, since
 * k7 is intended to be as much a standard as a
 * useful reference implementation. — isaacs
 **/

#include <k7.h>

#include <stdlib.h>
#include <time.h>
#include <string>

#define MODULE_NAME   "system.engine"
#define MODULE_STATIC  system_engine

// @TODO: Move this to system.engine
static Handle<Value> EvalCX (const Arguments& args)
{
	HandleScope handle_scope;
	Handle<Context> context = Context::New();
	context->SetSecurityToken(
		Context::GetCurrent()->GetSecurityToken()
	);
	
	Context::Scope context_scope(context);
	Handle<String> code = args[0]->ToString();
	Handle<Object> sandbox = args.Length() >= 1 ?
		args[1]->ToObject() : Context::GetCurrent()->Global();
	
	// share global datas
	OBJECT_COPY_SLOTS(context->Global(), sandbox);
	context->Enter();
	
	TryCatch try_catch;
	Handle<Script> script = Script::Compile(code, String::New("evalcx"));
	Handle<Value> result;
	if (script.IsEmpty()) {
		//ReportException(&try_catch);
		result = ThrowException(try_catch.Exception());
		goto CX_CLEANUP; // no run for you!
	}
	
	result = script->Run();
	if (result.IsEmpty()) {
		//ReportException(&try_catch);
		result = ThrowException(try_catch.Exception());
		goto CX_CLEANUP; // no global copy for you!
	}
	
	if (args.Length() >= 3 && args[2]->IsTrue()) {
		OBJECT_COPY_SLOTS(context->Global(), sandbox)
	}
	
CX_CLEANUP:
	// Because the global is about to be destroyed,
	// if the code ended in some reference to the global
	// it'll be "null" in the calling code, when the
	// more intuitive intent would be to return a reference
	// to the sandbox object.
	if (result == context->Global()) {
		result = sandbox;
	}
	context->DetachGlobal();
	context->Exit();
	return result;
}

FUNCTION(getState)
{
	STUB
}
END

FUNCTION(setState)
{
	STUB
}
END

FUNCTION(compileScript)
{
	STUB
}
END

FUNCTION(runScript)
{
	STUB
}
END

FUNCTION(defineProperty)
{
	STUB
}
END

FUNCTION(getPropertyDefinition)
{
	STUB
}
END

FUNCTION(sealObject)
{
	STUB
}
END

FUNCTION(freezeObject)
{
	STUB
}
END

MODULE
{
	BIND("getState", getState);
	BIND("setState", setState);
	BIND("compileScript", compileScript);
	BIND("runScript", runScript);
	BIND("defineProperty", defineProperty);
	BIND("getPropertyDefinition", getPropertyDefinition);
}
END_MODULE
