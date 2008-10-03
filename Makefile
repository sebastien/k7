PRODUCT               =k7

CPP                   =g++
BUILD_DIR             =build
BUILD_LIBS            =-lpthread -ldl 
V8_INCLUDE            =deps/v8/include
V8_BINARY             =deps/v8/libv8.a
BUILD_BINLIBS         =$(V8_BINARY) deps/shttpd/src/libshttpd.a

SOURCES               =$(wildcard src/*.cpp)
SOURCES_API           =$(shell find lib -name "*.api")
HEADERS               =$(wildcard src/*.h)
MODULES               =$(wildcard lib/*.cpp lib/*/*.cpp lib/*/*/*.cpp lib/*/*/*/*.cpp)
OBJECTS               =$(SOURCES:src/%.cpp=build/%.o)
SOBJECTS              =$(MODULES:lib/%.cpp=build/%.o)
INCLUDES              =-I$(V8_INCLUDE) -Isrc -Ideps

all: $(OBJECTS) $(SOBJECTS) $(V8_BINARY)
	g++ $(INCLUDES) $(OBJECTS) $(SOBJECTS) -o $(PRODUCT) $(BUILD_BINLIBS) $(BUILD_LIBS)

info:
	@echo Modules: $(MODULES)
	@echo Sources: $(SOURCES)
	@echo API:     $(SOURCES_API)

api: k7-api.html
	

build:
	mkdir build

deps:
	mkdir deps

deps/v8:
	cd deps && svn checkout http://v8.googlecode.com/svn/branches/bleeding_edge/ v8

deps/shttpd:
	cd deps && wget 'http://voxel.dl.sourceforge.net/sourceforge/shttpd/shttpd-1.42.tar.gz' && tar fvxz shttpd-1.42.tar.gz && rm shttpd-1.42.tar.gz && mv shttpd-1.42 shttpd

deps/v8/libv8.a: deps/v8
	cd deps/v8 && scons

build/%.o: src/%.cpp $(HEADERS) build deps/v8
	g++ $(INCLUDES) -c $< -o $@

#g++ $(INCLUDES) -fPIC $< -o $@ $(V8_BINARY) $(BUILD_LIBS)
build/%.o: lib/%.cpp $(HEADERS) build
	@mkdir -p `dirname $@` || true
	g++ $(INCLUDES) -c $< -o $@

k7-api.html: $(SOURCES_API)
	sugar -a$@ -ljs $(SOURCES_API)

# EOF
