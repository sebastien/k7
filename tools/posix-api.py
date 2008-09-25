__doc__ = """
Extracts POSIX function prototypes from the POSIX man pages and generates a C++
that wraps the POSIX functions in V8.
"""

# 1 - Extract prototypes and create types catalogue
# 2 - Create  V8 definitions for each POSIX type
# 3 - Create  wrapper functions

import os,re
NAME="[\&\*\(\)A-Za-z0-9_]+"
RE_FUNCTION=re.compile("\s*(%s)\s+([A-Za-z0-9_]+)\s*\(([^)]*)\)" % (NAME))
for line in file("posix-api.data"):
	function = line.strip()[:-2]
	man      = os.popen("man -Pcat %s" % (function)).read()
	print   len(man)
	text     = man[man.find("SYNOPSIS")+len("SYNOPSIS"):man.find("DESCRIPTION")]
	line     = map(lambda l:RE_FUNCTION.match(l), filter(lambda l:l.find(function) != -1,text.split("\n")))[0]
	if not line:
		print "Problem with ", function
	else:
		print (line.group(2), line.group(1), line.group(3))
# EOF
