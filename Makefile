PRODUCT               =k7

CPP                   =g++
BUILD_DIR             =build
BUILD_LIBS            =-lpthread -ldl
V8_INCLUDE            =deps/v8/include
V8_BINARY             =deps/v8/libv8.a

SOURCES               =$(wildcard src/*.cpp)
MODULES               =$(wildcard lib/*.cpp lib/*/*.cpp lib/*/*/*.cpp)
OBJECTS               =$(SOURCES:src/%.cpp=build/%.o)
SOBJECTS              =$(MODULES:lib/%.cpp=build/%.o)
INCLUDES              =-I$(V8_INCLUDE) -Isrc

all: $(OBJECTS) $(SOBJECTS) $(V8_BINARY)
	g++ $(INCLUDES) $(OBJECTS) $(SOBJECTS) -o $(PRODUCT) $(V8_BINARY) $(BUILD_LIBS)

info:
	@echo Modules: $(MODULES)
	@echo Sources: $(SOURCES)

build:
	mkdir build

deps:
	mkdir deps

deps/v8: deps
	cd deps && svn checkout http://v8.googlecode.com/svn/branches/bleeding_edge/ v8

deps/v8/libv8.a: deps/v8
	cd deps/v8 && scons

build/%.o: src/%.cpp build deps/v8
	g++ $(INCLUDES) -c $< -o $@

#g++ $(INCLUDES) -fPIC $< -o $@ $(V8_BINARY) $(BUILD_LIBS)
build/%.o: lib/%.cpp build
	@mkdir -p `dirname $@` || true
	g++ $(INCLUDES) -c $< -o $@

# EOF
