
BUILD_DIR             =build
CPP                   =g++
INCLUDE_V8            =archives/v8-read-only/include
BINLIB_V8             =archives/v8-read-only/libv8.a
LIBS                  =-lpthread -ldl
INCLUDES              =-I$(INCLUDE_V8) -Isrc
PRODUCT               =k7

SOURCES               =$(wildcard src/*.cpp)
OBJECTS               =$(SOURCES:src/%.cpp=build/%.o)


all: $(OBJECTS)
	g++ $(INCLUDES) build/modules.o build/posix.o build/main.o -o $(PRODUCT) $(BINLIB_V8) $(LIBS)

build:
	mkdir build

build/%.o: src/%.cpp build
	g++ $(INCLUDES) -c $< -o $@


