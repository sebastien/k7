// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
// Author            : Isaac Schulter.                           <i@foohack.com>
// ----------------------------------------------------------------------------
// Creation date     : 27-Sep-2008
// Last modification : 08-May-2009
// ----------------------------------------------------------------------------

#ifndef __K7_H__
#define __K7_H__

#include <v8.h>
#include "macros.h"
#include <assert.h>

void k7_reportException (const TryCatch* try_catch);
bool k7_evalString (Handle<String> source, Handle<Value> fromFileName);
int  k7_main (int argc, char **argv, char **env);

#endif
// EOF - vim: ts=4 sw=4 noet
