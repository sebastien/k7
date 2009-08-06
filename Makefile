# K7 Build system (2009-06-12)
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
DEBUG             =0
STATIC            =0

# Modules
CURL              =$(shell locate include/curl/curl.h)
FCGI              =$(shell locate include/fastcgi.h)
LIBEVENT          =0
LIBTASK           =0

ifeq  ($(PLATFORM),Darwin)
	LIB_ICONV          = -liconv
	BUILD_LIBS        += $(LIB_ICONV)
	# We disable the DYNAMIC (shared library) feature on Darwin
	STATIC             = 1
endif

ifeq ($(DEBUG),1)
	CPPFLAGS          += -g
endif

ifeq ($(STATIC),1)
	CPPFLAGS          += -DSTATIC
	SOBJECTS           =$(MODULES:lib/%.cpp=build/%.o)
endif

ifneq ($(strip $(CURL)),)
	CPPFLAGS          +=-DWITH_CURL
	LIB_CURL           = -lcurl
	BUILD_LIBS        += $(LIB_CURL)
endif
ifneq ($(strip $(FCGI)),)
	CPPFLAGS          +=-DWITH_FCGI
	LIB_FCGI           = -lfcgi
	BUILD_LIBS        += $(LIB_FCGI)
endif

ifeq ($(LIBEVENT),1)
	# NOTE: We exepect libevent-2.0, which is not widely available,
	# so people willing to use a specific library .so or .a can redefine
	# the LIB_EVENT variable
	CPPFLAGS          +=-DWITH_LIBEVENT
	LIB_EVENT          = deps/libevent/.libs/libevent.a
	BUILD_LIBS        += $(LIB_EVENT)
endif

ifeq ($(LIBTASK),1)
	CPPFLAGS          += -DWITH_LIBTASK
	LIB_TASK           = deps/libtask/libtask.a
	BUILD_BINLIBS     += $(LIB_TASK)
ifeq ($(STATIC),0)
	SOBJECTS      += build/core/concurrency/libtask/libtask.o
endif
endif


.PHONY: options info xinfo

k7: $(OBJECTS) $(SOBJECTS) $(PLUGINS) $(BUILD_BINLIBS) $(V8_BINARY)
	$(CPP) $(CPPFLAGS) $(INCLUDES) $(OBJECTS) $(SOBJECTS) -rdynamic -o $(PRODUCT) $(BUILD_BINLIBS) $(BUILD_LIBS)

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
	@echo "Options:\nCURL=$(CURL) FCGI=$(FCGI) LIBEVENT=$(LIBEVENT) LIBTASK=$(LIBTASK)\n"
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
	@echo "LIBEVENT - Enables libevent2 bindings (default=1, requires event2/event.h)"
	@echo "LIBTASK  - Enables libtask bindings  (default=0)"

api: doc/k7-api.html
	

compact: k7
	strip $<
	upx --version && upx $<

clean:
	rm -rf build

build:
	mkdir build

deps:
	mkdir deps


deps/v8:
	#cd deps && svn checkout http://v8.googlecode.com/svn/trunk v8
	cd deps && svn checkout http://v8.googlecode.com/svn/branches/bleeding_edge v8

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
