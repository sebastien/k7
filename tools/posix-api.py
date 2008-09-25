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
RE_SPACES=re.compile("\s+")

def getFunctionDescription( function ):
	man      = os.popen("man -Pcat 3 %s" % (function)).read()
	print   len(man)
	text     = man[man.find("SYNOPSIS")+len("SYNOPSIS"):man.find("DESCRIPTION")]
	line     = map(lambda l:RE_FUNCTION.match(l), filter(lambda l:l.find(function) != -1,text.split("\n")))[0]
	if not line:
		i = text.find(function+"(")
		text  = text[i:text.find(")",i+1)+1].replace("\n"," ").replace("\t"," ")
		print text
		line  = RE_FUNCTION.match(text)
		print line
	if not line:
		print "Problem with ", function
	else:
		print (line.group(2), line.group(1), line.group(3))

for line in file("posix-api.data"):
	getFunctionDescription (line.replace("()","").strip())
#getFunctionDescription ("accept")

# EOF
