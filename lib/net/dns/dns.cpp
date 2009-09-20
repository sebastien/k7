// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
// -----------------------------------------------------------------------------
// Creation date     : 20-Sep-2009
// Last modification : 20-Sep-2009
// -----------------------------------------------------------------------------

#include <k7.h>
#include "node/src/dns.h"

LINK_TO(libdns.a)

#define MODULE_NAME   "net.dns"
#define MODULE_STATIC  net_dns

// ----------------------------------------------------------------------------
//
// MODULE DECLARATION
//
// ----------------------------------------------------------------------------

MODULE
{
	node::DNS::Initialize(__module__);
}
END_MODULE

// EOF - vim: ts=4 sw=4 noet
