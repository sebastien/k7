# TODO: Check for
# fastcgi.h - libfcgi-dev
# curl.h    - libcurl*-dev
PRODUCT               =k7

PLATFORM              =$(shell uname -r)
CPP                   =g++
CPPFLAGS              =
BUILD_DIR             =build
BUILD_LIBS            =-lpthread -ldl
# NOTE: On OSX, I think -liconv is necessary
BUILD_BINLIBS         =$(V8_BINARY) deps/shttpd/src/libshttpd.a

V8_INCLUDE            =deps/v8/include
V8_BINARY             =deps/v8/libv8.a
SOURCES               =$(wildcard src/*.cpp)
SOURCES_API           =$(shell find lib -name "*.api")
HEADERS               =$(wildcard src/*.h)
MODULES               =$(wildcard lib/*.cpp lib/*/*.cpp lib/*/*/*.cpp lib/*/*/*/*.cpp)
OBJECTS               =$(SOURCES:src/%.cpp=build/%.o)
SOBJECTS              =$(MODULES:lib/%.cpp=build/%.o)
INCLUDES              =-I$(V8_INCLUDE) -Isrc -Ideps

# Modules
HAS_CURL              =$(shell locate include/curl/curl.h)
HAS_FCGI              =$(shell locate include/fastcgi.h)
ifneq ($(strip $(HAS_CURL)),)
	CPPFLAGS          +=-DWITH_CURL
	BUILD_LIBS        +=-lcurl
endif
ifneq ($(strip $(HAS_FCGI)),)
	CPPFLAGS          +=-DWITH_FCGI
	BUILD_LIBS        +=-lfcgi
endif

all: $(OBJECTS) $(SOBJECTS) $(BUILD_BINLIBS) $(BUILD_LkIBS) $(V8_BINARY)
	$(CPP) $(CPPFLAGS) $(INCLUDES) $(OBJECTS) $(SOBJECTS) -o $(PRODUCT) $(BUILD_BINLIBS) $(BUILD_LIBS)

info:
	@echo Modules: $(MODULES)
	@echo Sources: $(SOURCES)
	@echo API:     $(SOURCES_API)

api: k7-api.html
	

build:
	mkdir build

clean:
	rm -rf build

deps:
	mkdir deps

deps/v8:
	cd deps && svn checkout http://v8.googlecode.com/svn/trunk v8

deps/mongoose:
	cd deps && svn checkout http://mongoose.googlecode.com/svn/trunk/ mongoose

deps/shttpd/src/libshttpd.a:
	cd deps/shttpd/src && make unix LIBS="-ldl -lpthread"


deps/shttpd:
	cd deps && wget 'http://voxel.dl.sourceforge.net/sourceforge/shttpd/shttpd-1.42.tar.gz' && tar fvxz shttpd-1.42.tar.gz && rm shttpd-1.42.tar.gz && mv shttpd-1.42 shttpd

deps/v8/libv8.a: deps/v8
	cd deps/v8 && scons

build/%.o: src/%.cpp $(HEADERS) build deps/v8
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

#g++ $(INCLUDES) -fPIC $< -o $@ $(V8_BINARY) $(BUILD_LIBS)
build/%.o: lib/%.cpp $(HEADERS) build
	@mkdir -p `dirname $@` || true
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

k7-api.html: $(SOURCES_API)
	sugar -a$@ -ljs $(SOURCES_API)

# EOF
