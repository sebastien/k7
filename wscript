#!/usr/bin/env python
# -----------------------------------------------------------------------------
# Project   : K7/Makefile
# -----------------------------------------------------------------------------
# Author    : Sebastien Pierre                           <sebastien@type-z.org>
# License   : BSD License
# -----------------------------------------------------------------------------
# Creation  : 14-Sep-2009
# Last mod  : 14-Sep-2009
# -----------------------------------------------------------------------------

# NOTE: This is largely borrowed from Ryan Dahl's node.js wscript

import re, sys, os, shutil ; sys.path.insert(0,"deps/tools")
import platform, logging, Options
from   os.path import join, dirname, abspath

VERSION  = "0.0.1"
APPNAME  = "k7"
CWD      = os.getcwd()

srcdir   = "."
blddir   = "build"
fatal    = logging.fatal

# -----------------------------------------------------------------------------
#
# UTILITIES
#
# -----------------------------------------------------------------------------

# XXX Remove this when v8 defaults x86_64 to native builds
def GuessArchitecture():
	id = platform.machine()
	if id.startswith('arm'):
		return 'arm'
	elif '64' in id:
		return 'x64'
	elif (not id) or (not re.match('(x|i[3-6])86', id) is None):
		return 'ia32'
	else:
		return None

def mkdir_p(dir):
	if not os.path.exists (dir):
		os.makedirs (dir)

# -----------------------------------------------------------------------------
#
# DEPENDENCIES TASKS
#
# -----------------------------------------------------------------------------

def conf_subproject (conf, subdir, command=None):
	print("---- %s ----" % subdir)
	src = join(conf.srcdir, subdir)
	if not os.path.exists (src):
		logging.fatal("no such subproject " + subdir)

	default_tgt = join(conf.blddir, "default", subdir)

	if not os.path.exists(default_tgt):
		shutil.copytree(src, default_tgt)

	if command:
		if os.system("cd %s && %s" % (default_tgt, command)) != 0:
			logging.fatal("Configuring %s failed." % (subdir))

	debug_tgt = join(conf.blddir, "debug", subdir)

	if not os.path.exists(debug_tgt):
		shutil.copytree(default_tgt, debug_tgt)

# -----------------------------------------------------------------------------
#
# UDNS LIBRARY
#
# -----------------------------------------------------------------------------

def build_udns(bld):
	default_build_dir = bld.srcnode.abspath(bld.env_of_name("default"))

	default_dir = join(default_build_dir, "deps/udns")

	static_lib = bld.env["staticlib_PATTERN"] % "udns"

	rule = 'cd %s && make'

	default = bld.new_task_gen(
		target= join("deps/udns", static_lib),
		rule= rule % default_dir,
		before= "cxx",
		install_path= None
	)

	bld.env["CPPPATH_UDNS"] = "deps/udns"
	bld.env["STATICLIB_UDNS"] = "udns"

	bld.env_of_name('default')["STATICLIB_UDNS"] = "udns"
	bld.env_of_name('default')["LIBPATH_UDNS"] = default_dir

	if bld.env["USE_DEBUG"]:
		debug_build_dir = bld.srcnode.abspath(bld.env_of_name("debug"))
		debug_dir = join(debug_build_dir, "deps/udns")
		debug = default.clone("debug")
		debug.rule = rule % debug_dir
		#debug.target = join(debug_dir, static_lib)
		bld.env_of_name('debug')["STATICLIB_UDNS"] = "udns"
		bld.env_of_name('debug')["LIBPATH_UDNS"] = debug_dir
	bld.install_files('${PREFIX}/include/node/', 'deps/udns/udns.h');

# -----------------------------------------------------------------------------
#
# V8
#
# -----------------------------------------------------------------------------

def build_v8(bld):
	deps_src = join(bld.path.abspath(),"deps")
	deps_tgt = join(bld.srcnode.abspath(bld.env_of_name("default")),"deps")
	v8dir_src = join(deps_src,"v8")
	v8dir_tgt = join(deps_tgt, "v8")
	scons = os.path.join(CWD, 'deps/tools/scons/scons.py')

	v8rule = 'cd %s && ' \
		'python %s -Q mode=%s %s library=static snapshot=on'

	arch = ""
	if GuessArchitecture() == "x64":
		arch = "arch=x64"

	# HACK FIXME - use 32bit on Mac
	if platform.system() == "Darwin": arch = "arch=ia32";

	v8 = bld.new_task_gen(
		target = join("deps/v8", bld.env["staticlib_PATTERN"] % "v8"),
		rule=v8rule % (v8dir_tgt, scons, "release", arch),
		before="cxx",
		install_path = None
	)
	bld.env["CPPPATH_V8"] = "deps/v8/include"
	bld.env_of_name('default')["STATICLIB_V8"] = "v8"
	bld.env_of_name('default')["LIBPATH_V8"] = v8dir_tgt
	bld.env_of_name('default')["LINKFLAGS_V8"] = ["-pthread"]

	# HACK FIXME - use 32bit on Mac
	if platform.system() == "Darwin":
		bld.env_of_name('default')["LINKFLAGS_V8"] = ["-pthread", "-m32"]

	### v8 debug
	if bld.env["USE_DEBUG"]:
		deps_tgt = join(bld.srcnode.abspath(bld.env_of_name("debug")),"deps")
		v8dir_tgt = join(deps_tgt, "v8")

		v8_debug = v8.clone("debug")
		bld.env_of_name('debug')["STATICLIB_V8"] = "v8_g"
		bld.env_of_name('debug')["LIBPATH_V8"] = v8dir_tgt
		bld.env_of_name('debug')["LINKFLAGS_V8"] = ["-pthread"]

		# HACK FIXME - use 32bit on Mac
		if platform.system() == "Darwin":
			bld.env_of_name('debug')["LINKFLAGS_V8"] = ["-pthread", "-m32"]

		v8_debug.rule = v8rule % (v8dir_tgt, scons, "debug", arch)
		v8_debug.target = join("deps/v8", bld.env["staticlib_PATTERN"] % "v8_g")

	bld.install_files('${PREFIX}/include/node/', 'deps/v8/include/v8*');

# -----------------------------------------------------------------------------
#
# WAF WORKFLOW
#
# -----------------------------------------------------------------------------

def init():
	pass

def set_options(opt):
	opt.tool_options('compiler_cxx')
	opt.tool_options('compiler_cc')
	opt.tool_options('misc')
	opt.add_option('--static'
		, action  = 'store_true'
		, default = True
		, help    = 'statically links modules into K7'
	)
	opt.add_option('--debug'
		, action  = 'store_true'
		, default = True
		, help    = 'turns on debugging mode'
	)


def configure(conf):
	conf.check_tool('compiler_cxx')
	conf.check_tool('compiler_cc')
	conf.check(lib='dl', uselib_store='DL')
	conf.env["USE_DEBUG"] = Options.options.debug
	conf.env.append_value("CCFLAGS",      "-rdynamic")
	conf.env.append_value("LINKFLAGS_DL", "-rdynamic")

	if sys.platform.startswith("freebsd"):
		if not conf.check(lib="execinfo", libpath=['/usr/lib', '/usr/local/lib'], uselib_store="EXECINFO"):
			logging.fatal("Install the libexecinfo port from /usr/ports/devel/libexecinfo.")

	conf.sub_config('deps/libeio')
	conf.sub_config('deps/libev')

	conf_subproject(conf, 'deps/udns', './configure')
	conf_subproject(conf, 'deps/v8')

	conf.define("HAVE_CONFIG_H", 1)
	conf.env.append_value("CCFLAGS", "-DX_STACKSIZE=%d" % (1024*64))
	conf.env.append_value("PYTHONPATH", "deps/tools")

	# Split off debug variant before adding variant specific defines
	debug_env = conf.env.copy()
	conf.set_env_name('debug', debug_env)

	# Configure debug variant
	conf.setenv('debug')
	debug_env.set_variant('debug')
	debug_env.append_value('CCFLAGS', ['-DDEBUG', '-g', '-O0', '-Wall', '-Wextra'])
	debug_env.append_value('CXXFLAGS', ['-DDEBUG', '-g', '-O0', '-Wall', '-Wextra'])

	# HACK FIXME - use 32bit on Mac
	if platform.system() == "Darwin":
		debug_env.append_value('CCFLAGS', '-m32')
		debug_env.append_value('CXXFLAGS', '-m32')

	conf.write_config_header("config.h")

	# Configure default variant
	conf.setenv('default')
	conf.env.append_value('CCFLAGS', ['-DNDEBUG', '-O3'])
	conf.env.append_value('CXXFLAGS', ['-DNDEBUG', '-O3'])

	# HACK FIXME - use 32bit on Mac
	if platform.system() == "Darwin":
		conf.env.append_value('CCFLAGS', '-m32')
		conf.env.append_value('CXXFLAGS', '-m32')

	conf.write_config_header("config.h")


def build_evcom( bld ):
	### evcom
	evcom = bld.new_task_gen("cc", "staticlib")
	evcom.source = "deps/evcom/evcom.c"
	evcom.includes = "deps/evcom/ deps/libev/"
	evcom.name = "evcom"
	evcom.target = "evcom"

	# evcom.uselib = "GNUTLS"
	evcom.install_path = None
	if bld.env["USE_DEBUG"]:
		evcom.clone("debug")
	bld.install_files('${PREFIX}/include/node/', 'deps/evcom/evcom.h');

def build_http_parser( bld ):
	### http_parser
	http_parser = bld.new_task_gen("cc", "staticlib")
	http_parser.source = "deps/http_parser/http_parser.c"
	http_parser.includes = "deps/http_parser/"
	http_parser.name = "http_parser"
	http_parser.target = "http_parser"
	http_parser.install_path = None
	if bld.env["USE_DEBUG"]:
		http_parser.clone("debug")
	
def build_coupling( bld ):
	### coupling
	coupling = bld.new_task_gen("cc", "staticlib")
	coupling.source = "deps/coupling/coupling.c"
	coupling.includes = "deps/coupling/"
	coupling.name = "coupling"
	coupling.target = "coupling"
	coupling.install_path = None
	if bld.env["USE_DEBUG"]:
		coupling.clone("debug")

def build_node( bld ):
	### src/native.cc
	import js2c, jsmin
	def javascript_in_c(task):
		env = task.env
		source = map(lambda x: x.srcpath(env), task.inputs)
		targets = map(lambda x: x.srcpath(env), task.outputs)
		js2c.JS2C(source, targets)

	native_cc = bld.new_task_gen(
		source = """
		  deps/node/util.js
		  deps/node/events.js
		  deps/node/http.js
		  deps/node/file.js
		  deps/node/node.js
		""",
		target="deps/node/natives.h",
		rule=javascript_in_c,
		before="cxx"
	)
	native_cc.install_path = None
	if bld.env["USE_DEBUG"]:
		native_cc.clone("debug")
	### node lib
	node = bld.new_task_gen("cxx", "staticlib")
	node.name         = "node"
	node.target       = "node"
	node.source = """
		deps/node/node.cc
		deps/node/events.cc
		deps/node/http.cc
		deps/node/net.cc
		deps/node/node_stdio.cc
		deps/node/dns.cc
		deps/node/file.cc
		deps/node/timer.cc
		deps/node/child_process.cc
		deps/node/constants.cc
	"""
	node.includes = """
		deps/node 
		deps/v8/include
		deps/libev
		deps/udns
		deps/libeio
		deps/evcom 
		deps/http_parser
		deps/coupling
	"""
	node.uselib_local = "evcom ev eio http_parser coupling"
	node.uselib       = "UDNS V8 EXECINFO DL"
	node.chmod        = 0755

	def subflags(program):
		x = { 'CCFLAGS'   : " ".join(program.env["CCFLAGS"])
		    , 'CPPFLAGS'  : " ".join(program.env["CPPFLAGS"])
		    , 'LIBFLAGS'  : " ".join(program.env["LIBFLAGS"])
		    , 'VERSION'   : VERSION
		    , 'PREFIX'    : program.env["PREFIX"]
		}
		return x;

	# process file.pc.in -> file.pc
	pkgconfig              = bld.new_task_gen('subst', before="cxx")
	pkgconfig.source       =  'deps/node/node.pc.in'
	pkgconfig.target       = 'node.pc'
	pkgconfig.install_path = '${PREFIX}/lib/pkgconfig'
	pkgconfig.dict         = subflags(node)

	# process file.pc.in -> file.pc
	node_version = bld.new_task_gen('subst', before="cxx")
	node_version.source       = 'deps/node/node_version.h.in'
	node_version.target       = 'deps/node/node_version.h'
	node_version.dict         = subflags(node)
	node_version.install_path = '${PREFIX}/include/node'

	if bld.env["USE_DEBUG"]:
		node_g = node.clone("debug")
		node_g.target = "node_g"
		
		node_version_g = node_version.clone("debug")
		node_version_g.dict = subflags(node_g)
		node_version_g.install_path = None

def build_k7( bld ):
	k7 = bld.new_task_gen("cxx", "program")
	k7.name         = "k7"
	k7.target       = "k7"
	k7.source       = """
		src/k7.cpp
	"""
	k7.includes     = """
		src/
		deps/coupling
		deps/evcom
		deps/http_parser
		deps/libeio
		deps/libev
		deps/udns
		deps/libtask
		deps/shttpd
		deps/node/src
		deps/v8/include
	"""
	k7.uselib_local = "evcom ev eio http_parser coupling node"
	k7.uselib       = "UDNS V8 EXECINFO DL"
	k7.chmod        = 0755
	if bld.env["USE_DEBUG"]:
		k7_g = k7.clone("debug")
		k7_g.target = "node_g"

def build(bld):
	print('  building the project')
	bld.add_subdirs('deps/libeio deps/libev')
	build_udns(bld)
	build_v8(bld)
	build_evcom(bld)
	build_http_parser(bld)
	build_coupling(bld)
	build_node(bld)
	build_k7(bld)

def shutdown():
	pass


# EOF - vim: syn=python
