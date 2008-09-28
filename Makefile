PRODUCT               =k7

CPP                   =g++
BUILD_DIR             =build
BUILD_LIBS            =-lpthread -ldl
V8_INCLUDE            =archives/v8-read-only/include
V8_BINARY             =archives/v8-read-only/libv8.a

SOURCES               =$(wildcard src/*.cpp)
MODULES               =$(wildcard lib/*.cpp lib/*/*.cpp lib/*/*/*.cpp)
OBJECTS               =$(SOURCES:src/%.cpp=build/%.o)
SOBJECTS              =$(MODULES:lib/%.cpp=build/%.o)
INCLUDES              =-I$(V8_INCLUDE) -Isrc

all: $(OBJECTS) $(SOBJECTS)
	g++ $(INCLUDES) $(OBJECTS) -o $(PRODUCT) $(V8_BINARY) $(BUILD_LIBS)

info:
	@echo Modules: $(MODULES)
	@echo Sources: $(SOURCES)

build:
	mkdir build

build/%.o: src/%.cpp build
	g++ $(INCLUDES) -c $< -o $@

build/%.o: lib/%.cpp build
	@mkdir -p `dirname $@` || true
	g++ $(INCLUDES) -fPIC $< -o $@ $(V8_BINARY) $(BUILD_LIBS)


