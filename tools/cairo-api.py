from pygccxml import parser, declarations
decls = parser.parse(["/usr/include/cairo/cairo.h"])
ns    = decls[0]

def list_types(ns):
	types = {}
	for d in ns.decls():
		types[d.__class__.__name__] = True
	return types.keys()

def like(v,t):
	return v.__class__.__name__.find(t.lower()) != -1

class Library:

	def __init__( self ):
		self.types=[]
		self.functions={}

	def addFunction( self, f ):
		res  = []
		res.append(self.addType(str(d.return_type)))
		for a in d.arguments:
			res.append(self.addType(str(a.type)))
		if d.has_ellipsis: res.append("...")
		self.functions[d.name] =  res
		return res

	def addType( self, t ):
		nt = t.replace("*"," ").strip()
		if nt not in self.types:
			self.types.append(nt)
		return t

lib     = Library()
classes = {}
for d in ns.decls():
	if like(d,"funct"):
		if d.name.startswith("__"): continue
		args = lib.addFunction(d)
		if len(args) > 1:
			typ = args[1]
			res = classes.setdefault(typ,{})
			res[d.name] = args
for name in classes:
	print "class", name
	for function in classes[name]:
		print "    - ", function, classes[name][function]

