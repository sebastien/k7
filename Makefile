# K7 Build system (2009-09-20)
# ============================================================================
# I know, Make is not the best option, but it seems to work OK so far and the
# makefile is simple enough so that people can jump in without too much
# trouble.

# TODO: Check for
# fastcgi.h - libfcgi-dev
# curl.h    - libcurl*-dev

PRODUCT               =k7
VERSION               :=$(shell date +'%Y%m%d')

PLATFORM              =$(shell uname -s)
CPP                   =g++
CPPFLAGS              =
CPPFLAGS              += -DK7_VERSION=$(VERSION)
BUILD_DIR             =build
BUILD_LIBS            =-lpthread -ldl
# NOTE: On OSX, I think -liconv is necessary, search for Darwin in Makefile
BUILD_BINLIBS         =$(V8_BINARY) deps/shttpd/src/libshttpd.a

V8_INCLUDE            =deps/v8/include
V8_BINARY             =deps/v8/libv8.a
JS2H                  =tools/js2h
SOURCES               =$(wildcard src/*.cpp)
SOURCES_API           =$(shell find lib -name "*.api")
HEADERS               =$(wildcard src/*.h)
MODULES               =$(wildcard lib/*.cpp lib/*/*.cpp lib/*/*/*.cpp lib/*/*/*/*.cpp)
MODULES_JS            =$(wildcard lib/*.js lib/*/*.js lib/*/*/*.js lib/*/*/*/*.js)
MODULES_JS_H          =$(MODULES_JS:lib/%.js=build/include/%.js.h)
OBJECTS               =$(SOURCES:src/%.cpp=build/%.o)
SOBJECTS              =
PLUGINS               =$(MODULES:lib/%.cpp=build/plugins/%.so)
INCLUDES              =-I$(V8_INCLUDE) -Isrc -Ideps

# Options
DEBUG                 =1
STATIC                =1

# Modules
CURL                  =$(shell locate include/curl/curl.h)
FCGI                  =$(shell locate include/fastcgi.h)
LIBTASK               =1
LIBNODE               =1

ifeq  ($(PLATFORM),Darwin)
	LIB_ICONV         := -liconv
	BUILD_LIBS        += $(LIB_ICONV)
	# We disable the DYNAMIC (shared library) feature on Darwin
	STATIC            := 1
endif

ifeq ($(DEBUG),1)
	CPPFLAGS          += -g
endif

ifeq ($(STATIC),1)
	CPPFLAGS          += -DSTATIC
	SOBJECTS           =$(MODULES:lib/%.cpp=build/%.o)
	PLUGINS            =
endif

ifneq ($(strip $(CURL)),)
	CPPFLAGS          +=-DWITH_CURL
	LIB_CURL          := -lcurl
	BUILD_LIBS        += $(LIB_CURL)
endif
ifneq ($(strip $(FCGI)),)
	CPPFLAGS          +=-DWITH_FCGI
	LIB_FCGI          := -lfcgi
	BUILD_LIBS        += $(LIB_FCGI)
endif

ifeq ($(LIBTASK),1)
	CPPFLAGS          += -DWITH_LIBTASK
	LIB_TASK          := deps/libtask/libtask.a
	BUILD_BINLIBS     += $(LIB_TASK)
ifeq ($(STATIC),0)
	SOBJECTS      += build/core/concurrency/libtask/libtask.o
endif
endif

ifeq ($(LIBNODE),1)
	CPPFLAGS          +=-DWITH_LIBNODE -DEV_MULTIPLICITY=0
	LIB_NODE          := deps/node/build/default/libnode.a
	INCLUDES          += -Ideps/node -Ideps/node/src -Ideps/node/deps/coupling 
	INCLUDES          += -Ideps/node/deps/evcom -Ideps/node/deps/http_parser -Ideps/node/deps/libeio -Ideps/node/deps/libev -Ideps/node/deps/udns
	INCLUDES          += -Ideps/node/build/default/src
	NODE_MODE         :=default
ifeq ($(DEBUG),1)
	NODE_OPTIONS      += --debug
	NODE_MODE         :=debug
endif
	# FIXME: For some strange reason, I have to add '-lev', even if the libev.a is given, otherwise
	# I get the following error:
	# build/k7.o: In function `ev_default_loop_uc':
	# /home/sebastien/Projects/WIP/K7/deps/node/deps/libev/ev.h:449: undefined reference to `ev_default_loop_ptr'
	# build/k7.o: In function `ev_default_loop':
	# /home/sebastien/Projects/WIP/K7/deps/node/deps/libev/ev.h:463: undefined reference to `ev_default_loop_init'
	# collect2: ld a retourné 1 code d'état d'exécution
	#BUILD_LIBS        += -lev
	BUILD_BINLIBS     += $(LIB_NODE) \
	                     deps/node/build/$(NODE_MODE)/libevcom.a \
	                     deps/node/build/$(NODE_MODE)/libhttp_parser.a \
	                     deps/node/build/$(NODE_MODE)/libcoupling.a \
	                     deps/node/build/$(NODE_MODE)/deps/libev/libev.a \
	                     deps/node/build/$(NODE_MODE)/deps/udns/libudns.a \
	                     deps/node/build/$(NODE_MODE)/deps/libeio/libeio.a
endif


.PHONY: options info xinfo

k7: $(OBJECTS) $(SOBJECTS) $(PLUGINS) $(BUILD_BINLIBS) $(V8_BINARY)
	$(CPP) $(CPPFLAGS) $(INCLUDES) $(OBJECTS) $(SOBJECTS) -rdynamic -o $(PRODUCT) $(BUILD_BINLIBS) $(BUILD_LIBS)

sconfig.mk: sconfig.conf
	./tools/sconfig --conf=$< --output=$@

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
-include tools/sconfig.mk
endif
endif

info:
	@echo "K7 build system"
	@echo
	@echo "k7      - builds the ./k7 binary"
	@echo "compact - compacts the ./k7 binary (requires uxp)"
	@echo "api     - builds the API documentation (requires sdoc)"
	@echo "clean   - cleans the build system"
	@echo "options - displays available configuration options"
	@echo
	@echo "NOTE: build requires svn, scons and python in addition to gcc"

xinfo:
	@echo "Options:\nCURL=$(CURL) FCGI=$(FCGI) LIBNODE=$(LIBNODE) LIBTASK=$(LIBTASK)\n"
	@echo "Modules (native):\n$(MODULES)\n"
	@echo "Modules (js):\n$(MODULES_JS)\n"
	@echo "Sources:\n$(SOURCES)\n"
	@echo "Plugins:\n$(PLUGINS)\n"
	@echo "API:\n$(SOURCES_API)\n"

options:
	@echo "K7 build options"
	@echo
	@echo "CURL     - Enables curl bindings     (default=1, requires curl.h)"
	@echo "FCGI     - Enables FCGI bindings     (default=1, requires fastcgi.h)"
	@echo "LIBNODE  - Enables libnode bindings  (default=1)"
	@echo "LIBTASK  - Enables libtask bindings  (default=0)"

api: doc/k7-api.html
	

compact: k7
	strip $<
	upx --version && upx $<

clean:
	rm -rf build

veryclean: clean
	rm -rf deps/v8 deps/node deps/mongoose deps/shttpd deps/libevent

build:
	mkdir build

deps:
	mkdir deps

deps/v8:
	cd deps && svn checkout http://v8.googlecode.com/svn/branches/bleeding_edge v8

deps/node: deps/v8
	cd deps && git clone git://github.com/ry/node.git && cd node && rm -rf v8 && ln -s ../v8 . \

deps/mongoose:
	cd deps && svn checkout http://mongoose.googlecode.com/svn/trunk/ mongoose

deps/shttpd:
	cd deps && wget 'http://voxel.dl.sourceforge.net/sourceforge/shttpd/shttpd-1.42.tar.gz' && tar fvxz shttpd-1.42.tar.gz && rm shttpd-1.42.tar.gz && mv shttpd-1.42 shttpd

deps/libevent:
	cd deps && wget 'http://www.monkey.org/~provos/libevent-2.0.1-alpha.tar.gz' && tar xzvf libevent-2.0.1-alpha.tar.gz && rm libevent-2.0.1-alpha.tar.gz && mv libevent-2.0.1-alpha libevent
	#cd deps && svn co https://levent.svn.sourceforge.net/svnroot/trunk/libevent libevent

deps/v8/libv8.a: deps/v8
	cd deps/v8 && scons snapshot=on mode=release

deps/libtask/libtask.a: deps/libtask
	cd deps/libtask && make

deps/libevent/.libs/libevent.a: deps/libevent
	cd deps/libevent && ./configure && make

deps/shttpd/src/libshttpd.a:
	cd deps/shttpd/src && make unix LIBS="-ldl -lpthread"

deps/node/Makefile: deps/node
	sed -i 's|node = bld.new_task_gen("cxx", "program")|node = bld.new_task_gen("cxx", "staticlib")|g' deps/node/wscript
	sed -i 's|int main|int node_main|g' deps/node/src/node.cc
	cd deps/node && ./configure

deps/node/build/default/libnode.a: deps/node deps/node/Makefile
	cd deps/node && ./tools/waf --debug

build/%.o: src/%.cpp $(HEADERS) build deps/v8
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

#g++ $(INCLUDES) -fPIC $< -o $@ $(V8_BINARY) $(BUILD_LIBS)
build/%.o: lib/%.cpp  $(HEADERS)
	@mkdir -p `dirname $@` || true
	@# Using ifeq or sh if did not work... had to resort to this :/
	@test -e lib/$*.js && mkdir -p `dirname build/include/$*` || true
	@test -e lib/$*.js && $(JS2H) lib/$*.js > build/include/$*.js.h || true
	$(CPP) $(CPPFLAGS) $(INCLUDES) -Ibuild/include/$(dir $*) -c $< -o $@

build/plugins/%.o: lib/%.cpp  $(HEADERS)
	@mkdir -p `dirname $@` || true
	@# Using ifeq or sh if did not work... had to resort to this :/
	@test -e lib/$*.js && mkdir -p `dirname build/include/$*` || true
	@test -e lib/$*.js && $(JS2H) lib/$*.js > build/include/$*.js.h || true
	$(CPP) $(CPPFLAGS) $(INCLUDES) -fPIC -DAS_PLUGIN -Ibuild/include/$(dir $*) -c $< -o $@

build/plugins/%.so: build/plugins/%.o
	$(CPP) $(CPPFLAGS) -shared -o $@ $<
	strip $@

build/include/%.js.h: lib/%.js $(JS2H)
	@mkdir -p `dirname $@` || true
	$(JS2H) $< > $@

doc/k7-api.html: $(SOURCES_API)
	@mkdir -p `dirname $@` || true
	sugar -a$@ -ljs $(SOURCES_API)

# EOF
