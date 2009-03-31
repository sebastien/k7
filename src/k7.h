#ifndef __K7_H__
#define __K7_H__

#include <v8.h>
#include "macros.h"
#include "modules.h"
#include <assert.h>

// some of these might not be strictly necessary here.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <time.h>

bool ExecuteString(v8::Handle<v8::String> source, v8::Handle<v8::Value> name, bool print_result);

#endif
